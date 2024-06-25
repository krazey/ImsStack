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
#include "CarrierConfig.h"
#include "Engine.h"
#include "IConfiguration.h"
#include "Ims3gpp.h"
#include "ImsAosReason.h"
#include "MockIMtcCallController.h"
#include "MockIMtcService.h"
#include "MtcDef.h"
#include "PlatformContext.h"
#include "TestConfigService.h"
#include "call/IMtcCall.h"
#include "call/MockEpsFallbackTrigger.h"
#include "call/MockIMtcCallContext.h"
#include "call/MockIMtcCallManager.h"
#include "call/MockIMtcSession.h"
#include "call/MockIMtcUiNotifier.h"
#include "call/MockISilentRedialHelper.h"
#include "call/TestMtcPendingOperationHolder.h"
#include "call/extension/MockIMtcExtension.h"
#include "call/extension/MtcExtensionSet.h"
#include "call/state/MtcCallState.h"
#include "call/state/OutgoingState.h"
#include "configuration/MockIMtcConfigurationManager.h"
#include "configuration/MtcConfigurationProxy.h"
#include "core/ISession.h"
#include "core/MockIMessage.h"
#include "core/MockISession.h"
#include "dialingplan/MockIMtcDialingPlan.h"
#include "helper/ISrvccStateListener.h"
#include "helper/MockIMtcAosConnector.h"
#include "helper/MockMtcTimerWrapper.h"
#include "helper/MockUdpKeepAliveSender.h"
#include "helper/MtcSupplementaryService.h"
#include "helper/sipinterfaceholder/MockIInterfaceHolderListener.h"
#include "helper/sipinterfaceholder/MockIMtcSipInterfaceFactory.h"
#include "helper/sipinterfaceholder/MockSessionInterfaceHolder.h"
#include "media/MockIMtcMediaManager.h"
#include "precondition/MockIMtcPreconditionManager.h"
#include "precondition/QosDef.h"
#include "sipcore/ISipHeader.h"
#include "sipcore/MockISipMessage.h"
#include "sipcore/SipStatusCode.h"
#include "utility/MockIMessageUtils.h"
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;

static const IMS_CHAR SESSION_ID[] = "f81d4fae7dec11d0a76500a0c91e6bf6";

class OutgoingStateTest : public ::testing::Test
{
public:
    OutgoingState* pOutgoingState;
    MockIMtcSession objMtcSession;
    MockIMtcConfigurationManager* pConfigurationManager;
    MtcConfigurationProxy* pConfigurationProxy;
    MockIMtcCallContext objCallContext;
    MockIMtcService objService;
    MockIMtcAosConnector objAosConnector;
    MockISession objSession;
    MockIMtcMediaManager objMediaManager;
    MockIMtcPreconditionManager objPreconditionManager;
    MockIMtcCallController objController;
    MockISilentRedialHelper objRedialHelper;
    CallInfo objCallInfo;
    MockMtcTimerWrapper objTimer;
    MockIMtcUiNotifier objUiNotifier;
    MockIMessageUtils objMessageUtils;
    MockEpsFallbackTrigger* pEpsFbTrigger;
    MockIInterfaceHolderListener objInterfaceHolderListener;
    MockIMtcSipInterfaceFactory objSipInterfaceFactory;
    MockSessionInterfaceHolder* pSessionInterfaceHolder;
    MockIMtcExtension objExtension;
    SipMethod objAckMethod;
    SipMethod objInviteMethod;
    MtcSupplementaryService* pSupplementaryService;
    MediaInfo objMediaInfo;
    ImsList<IMtcSession*> objSessions;
    MockIMtcCallManager objCallManager;
    TestConfigService* pConfigService;

protected:
    virtual void SetUp() override
    {
        pConfigService = new TestConfigService();
        pConfigService->SetCarrierConfig(&(pConfigService->GetMockCarrierConfig()));
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_CONFIG, pConfigService);

        objAckMethod = SipMethod::ACK;
        objInviteMethod = SipMethod::INVITE;

        pConfigurationManager = new MockIMtcConfigurationManager();
        pConfigurationProxy = new MtcConfigurationProxy(pConfigurationManager);
        ON_CALL(objCallContext, GetConfigurationProxy)
                .WillByDefault(ReturnRef(*pConfigurationProxy));

        pSupplementaryService = new MtcSupplementaryService(objCallContext, *pConfigurationProxy);
        ON_CALL(objCallContext, GetSupplementaryService)
                .WillByDefault(ReturnRef(*pSupplementaryService));

        ON_CALL(objCallContext, GetCallController).WillByDefault(ReturnRef(objController));
        ON_CALL(objController, GetRedialHelper).WillByDefault(ReturnRef(objRedialHelper));
        ON_CALL(objCallContext, GetUiNotifier).WillByDefault(ReturnRef(objUiNotifier));

        objSessions.Append(&objMtcSession);
        ON_CALL(objCallContext, GetSessions()).WillByDefault(ReturnRef(objSessions));
        ON_CALL(objCallContext, GetSession()).WillByDefault(Return(&objMtcSession));
        ON_CALL(objCallContext, GetSession(_)).WillByDefault(Return(&objMtcSession));
        ON_CALL(objCallContext, GetCallInfo).WillByDefault(ReturnRef(objCallInfo));
        ON_CALL(objCallContext, GetService).WillByDefault(ReturnRef(objService));
        ON_CALL(objCallContext, GetTimer).WillByDefault(ReturnRef(objTimer));
        ON_CALL(objCallContext, GetMediaManager).WillByDefault(ReturnRef(objMediaManager));
        ON_CALL(objCallContext, GetMessageUtils).WillByDefault(ReturnRef(objMessageUtils));
        ON_CALL(objCallContext, GetCallManager).WillByDefault(ReturnRef(objCallManager));

        pEpsFbTrigger = new MockEpsFallbackTrigger(objCallContext);
        ON_CALL(objCallContext, GetEpsFallbackTrigger).WillByDefault(ReturnRef(*pEpsFbTrigger));

        ON_CALL(objService, GetAosConnector).WillByDefault(Return(&objAosConnector));

        ON_CALL(objMtcSession, GetISession).WillByDefault(ReturnRef(objSession));

        ON_CALL(objCallContext, GetPreconditionManager)
                .WillByDefault(ReturnRef(objPreconditionManager));

        pSessionInterfaceHolder = new MockSessionInterfaceHolder(objInterfaceHolderListener);
        ON_CALL(objSipInterfaceFactory, GetISessionHolder)
                .WillByDefault(Return(pSessionInterfaceHolder));
        ON_CALL(objCallContext, GetSipInterfaceFactory)
                .WillByDefault(ReturnRef(objSipInterfaceFactory));

        ON_CALL(objMediaManager, GetMediaInfo).WillByDefault(ReturnRef(objMediaInfo));
        ON_CALL(objMediaManager, GetRemoteRtpPort(_, MEDIATYPE_AUDIO)).WillByDefault(Return(12345));

        pOutgoingState = new OutgoingState(objCallContext);
    }

    virtual void TearDown() override
    {
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_CONFIG, IMS_NULL);

        delete pConfigService;
        delete pOutgoingState;
        delete pConfigurationProxy;
        delete pSupplementaryService;
        delete pEpsFbTrigger;
        delete pSessionInterfaceHolder;
    }

    void SetSdpOaFailure(IN MockIMessage& objMessage)
    {
        ON_CALL(objMessageUtils, HasSdp(&objMessage)).WillByDefault(Return(IMS_FALSE));
        ON_CALL(objMediaManager, GetNegotiationState(&objSession))
                .WillByDefault(Return(NegotiationState::STATE_OFFER_SENT));
        ON_CALL(objMessage, GetMethod).WillByDefault(ReturnRef(objAckMethod));
    }

    void SetSdpOaSuccessWithSdp(IN MockIMessage& objMessage, IN MockISipMessage& objSipMessage)
    {
        ON_CALL(objMessageUtils, HasSdp(&objMessage)).WillByDefault(Return(IMS_TRUE));
        ON_CALL(objSession, IsSdpNegotiationAllowedForNonRpr).WillByDefault(Return(IMS_FALSE));
        ON_CALL(objMessage, GetMethod).WillByDefault(ReturnRef(objInviteMethod));
        ON_CALL(objMessage, GetStatusCode).WillByDefault(Return(SipStatusCode::SC_183));
        ON_CALL(objMessage, GetMessage).WillByDefault(Return(&objSipMessage));
        ON_CALL(objSipMessage, IsMessageRpr).WillByDefault(Return(IMS_FALSE));
    }

    void SetUpStartErrorHandler(IN MockIMessage& objMessage, IN IMS_SINT32 nStatusCode,
            IN IMS_BOOL bCsfb, IN IMS_SINT32 nPolicyOfTimerB, IN IMS_BOOL bEpsFallbackRequired,
            IN IMS_BOOL bWiFi)
    {
        ON_CALL(objMessage, GetStatusCode).WillByDefault(Return(nStatusCode));
        ON_CALL(*pConfigurationManager, IsRejectCodeForCsfb(nStatusCode))
                .WillByDefault(Return(bCsfb));
        ON_CALL(objService, IsWlanIpCanType).WillByDefault(Return(bWiFi));
        ON_CALL(objService, IsEpsCombinedAttach).WillByDefault(Return(bCsfb));
        if (bWiFi)
        {
            ON_CALL(*pConfigurationManager, GetPolicyForTcallTimerExpiryOfVowifiCall)
                    .WillByDefault(Return(nPolicyOfTimerB));
        }
        else
        {
            ON_CALL(*pConfigurationManager, GetPolicyForTcallTimerExpiryOfVolteCall)
                    .WillByDefault(Return(nPolicyOfTimerB));
        }
        ON_CALL(*pConfigurationManager, GetEpsFallbackWatchdogTime)
                .WillByDefault(Return(bEpsFallbackRequired ? 2000 : -1));
        ON_CALL(objService, IsNr).WillByDefault(Return(bEpsFallbackRequired));
    }

    MtcExtensionSet* GetTestExtensionSet(IN const AString& strOptionTag)
    {
        ImsList<IMtcExtension*> objExtensions;
        ON_CALL(objExtension, GetOptionTag).WillByDefault(ReturnRef(strOptionTag));
        ON_CALL(objExtension, IsAvailableOnRemote).WillByDefault(Return(IMS_TRUE));
        objExtensions.Append(&objExtension);
        return new MtcExtensionSet(objCallContext, objExtensions);
    }
};

TEST_F(OutgoingStateTest, OnExitStopsUdpKeepAliveSenderIfSupported)
{
    MockUdpKeepAliveSender objKeepAliveSender(objCallContext);
    ON_CALL(objCallContext, GetUdpKeepAliveSender).WillByDefault(ReturnRef(objKeepAliveSender));
    ON_CALL(*pConfigurationManager, GetSendUdpKeepAliveIntervalTime).WillByDefault(Return(2000));
    EXPECT_CALL(objKeepAliveSender, Stop);
    pOutgoingState->OnExit();

    ON_CALL(*pConfigurationManager, GetSendUdpKeepAliveIntervalTime).WillByDefault(Return(0));
    EXPECT_CALL(objKeepAliveSender, Stop).Times(0);
    pOutgoingState->OnExit();
}

TEST_F(OutgoingStateTest, SessionPrackDeliveryFailedIgnoredIfConfigOn)
{
    ON_CALL(*pConfigurationManager, IsIgnorePrackDeliveryFailure).WillByDefault(Return(IMS_TRUE));

    EXPECT_EQ(CallStateName::OUTGOING, pOutgoingState->SessionPrackDeliveryFailed(&objSession));
}

TEST_F(OutgoingStateTest, HandleB1TimerIsNotHandledIfNoUserTerminateCase)
{
    EXPECT_CALL(objAosConnector, Control).Times(0);

    pOutgoingState->OnTimerExpired(MtcCallState::TIMER_MO_100_WAIT);
    const CallReasonInfo objReason(CODE_UNSPECIFIED);
    pOutgoingState->Terminate(objReason);
}

TEST_F(OutgoingStateTest, HandleB1TimerIsNotHandledIf100WaitTimerIsNotYetExpired)
{
    EXPECT_CALL(objAosConnector, Control).Times(0);

    const CallReasonInfo objReason(CODE_USER_TERMINATED);
    pOutgoingState->Terminate(objReason);
}

TEST_F(OutgoingStateTest, HandleB1TimerIsNotHandledIfPolicyIsNotWaitForResponse)
{
    EXPECT_CALL(objAosConnector, Control).Times(0);

    ON_CALL(objService, IsWlanIpCanType).WillByDefault(Return(IMS_FALSE));
    ON_CALL(*pConfigurationManager, GetPolicyForTcallTimerExpiryOfVolteCall)
            .WillByDefault(Return(CarrierConfig::ImsVoice::MO_CALL_REQUEST_TIMEOUT_POLICY_CSFB));

    pOutgoingState->OnTimerExpired(MtcCallState::TIMER_MO_100_WAIT);
    const CallReasonInfo objReason(CODE_USER_TERMINATED);
    pOutgoingState->Terminate(objReason);
}

TEST_F(OutgoingStateTest, HandleB1TimerIsNotHandledIfPolicyIsNotWaitForResponseInWifi)
{
    EXPECT_CALL(objAosConnector, Control).Times(0);

    ON_CALL(objService, IsWlanIpCanType).WillByDefault(Return(IMS_TRUE));
    ON_CALL(*pConfigurationManager, GetPolicyForTcallTimerExpiryOfVowifiCall)
            .WillByDefault(Return(CarrierConfig::ImsVoice::MO_CALL_REQUEST_TIMEOUT_POLICY_CSFB));

    pOutgoingState->OnTimerExpired(MtcCallState::TIMER_MO_100_WAIT);
    const CallReasonInfo objReason(CODE_USER_TERMINATED);
    pOutgoingState->Terminate(objReason);
}

TEST_F(OutgoingStateTest, HandleB1TimerIsHandled)
{
    EXPECT_CALL(objAosConnector, Control).Times(1);

    ON_CALL(objService, IsWlanIpCanType).WillByDefault(Return(IMS_FALSE));
    ON_CALL(*pConfigurationManager, GetPolicyForTcallTimerExpiryOfVolteCall)
            .WillByDefault(Return(
                    CarrierConfig::ImsVoice::MO_CALL_REQUEST_TIMEOUT_POLICY_WAIT_FOR_RESPONSE));

    pOutgoingState->OnTimerExpired(MtcCallState::TIMER_MO_100_WAIT);
    const CallReasonInfo objReason(CODE_USER_TERMINATED);
    pOutgoingState->Terminate(objReason);
}

TEST_F(OutgoingStateTest, HandleB1TimerIsHandledInWifi)
{
    EXPECT_CALL(objAosConnector, Control).Times(1);

    ON_CALL(objService, IsWlanIpCanType).WillByDefault(Return(IMS_TRUE));
    ON_CALL(*pConfigurationManager, GetPolicyForTcallTimerExpiryOfVowifiCall)
            .WillByDefault(Return(
                    CarrierConfig::ImsVoice::MO_CALL_REQUEST_TIMEOUT_POLICY_WAIT_FOR_RESPONSE));

    pOutgoingState->OnTimerExpired(MtcCallState::TIMER_MO_100_WAIT);
    const CallReasonInfo objReason(CODE_USER_TERMINATED);
    pOutgoingState->Terminate(objReason);
}

TEST_F(OutgoingStateTest, 18xTimerExpiredTerminatesCall)
{
    EXPECT_CALL(objMtcSession, Terminate(_, CallReasonInfo(CODE_TIMEOUT_1XX_WAITING)));
    EXPECT_CALL(objUiNotifier, SendStartFailed(CallReasonInfo(CODE_TIMEOUT_1XX_WAITING)));

    EXPECT_EQ(CallStateName::TERMINATING,
            pOutgoingState->OnTimerExpired(MtcCallState::TIMER_MO_18X_WAIT));
}

TEST_F(OutgoingStateTest, MoNoAnswerTimerExpiredTerminatesCall)
{
    EXPECT_CALL(objMtcSession, Terminate(_, CallReasonInfo(CODE_TIMEOUT_NO_ANSWER)));
    EXPECT_CALL(objUiNotifier, SendStartFailed(CallReasonInfo(CODE_TIMEOUT_NO_ANSWER)));

    EXPECT_EQ(CallStateName::TERMINATING,
            pOutgoingState->OnTimerExpired(MtcCallState::TIMER_MO_NOANSWER));
}

TEST_F(OutgoingStateTest, InvalidTimerExpiredDoesNothing)
{
    EXPECT_EQ(CallStateName::OUTGOING, pOutgoingState->OnTimerExpired(100));
}

TEST_F(OutgoingStateTest, OnMediaFailedReturnsTerminatingState)
{
    EXPECT_CALL(objPreconditionManager, FormPreconditionSdp).Times(0);

    EXPECT_CALL(objMtcSession, Terminate(_, CallReasonInfo(CODE_MEDIA_INIT_FAILED))).Times(1);

    EXPECT_CALL(objSession, GetPreviousResponse(_)).Times(0);

    EXPECT_EQ(pOutgoingState->OnMediaFailed(CallReasonInfo(CODE_MEDIA_INIT_FAILED)),
            CallStateName::TERMINATING);
}

TEST_F(OutgoingStateTest, OnMediaFailedReturnsOutgoingStateIfSessionIsNull)
{
    // TODO: let's check if keeping current state is correct
    ON_CALL(objCallContext, GetSession()).WillByDefault(Return(nullptr));
    EXPECT_EQ(pOutgoingState->OnMediaFailed(CallReasonInfo(CODE_MEDIA_INIT_FAILED)),
            CallStateName::OUTGOING);
}

TEST_F(OutgoingStateTest, OnIpcanChangedPushesPendingOperation)
{
    TestMtcPendingOperationHolder objPendingOperationHolder;
    ON_CALL(objCallContext, GetPendingOperationHolder)
            .WillByDefault(ReturnRef(objPendingOperationHolder));

    const IMS_UINT32 eIpcan = 1;
    EXPECT_CALL(objPendingOperationHolder.GetMock(), OnIpcanChanged(eIpcan));

    pOutgoingState->OnIpcanChanged(eIpcan);
}

TEST_F(OutgoingStateTest, SendUpdateBySrvccByCanceled)
{
    ON_CALL(objMediaManager, GetNegotiationState(_))
            .WillByDefault(Return(NegotiationState::STATE_NEGOTIATED));
    EXPECT_CALL(objMtcSession, SendEarlyUpdate(UpdateType::SRVCC_RECOVERED_CANCEL)).Times(1);

    EXPECT_EQ(CallStateName::OUTGOING, pOutgoingState->OnSrvccStateUpdated(SrvccState::CANCELED));
}

TEST_F(OutgoingStateTest, SendUpdateBySrvccByFailed)
{
    ON_CALL(objMediaManager, GetNegotiationState(_))
            .WillByDefault(Return(NegotiationState::STATE_NEGOTIATED));
    EXPECT_CALL(objMtcSession, SendEarlyUpdate(UpdateType::SRVCC_RECOVERED_FAILURE)).Times(1);

    EXPECT_EQ(CallStateName::OUTGOING, pOutgoingState->OnSrvccStateUpdated(SrvccState::FAILED));
}

TEST_F(OutgoingStateTest, SendUpdateBySrvccDoesNothingIfSessionIsNull)
{
    ON_CALL(objCallContext, GetSession()).WillByDefault(Return(nullptr));
    EXPECT_CALL(objMtcSession, SendEarlyUpdate(UpdateType::SRVCC_RECOVERED_FAILURE)).Times(0);

    EXPECT_EQ(CallStateName::OUTGOING, pOutgoingState->OnSrvccStateUpdated(SrvccState::FAILED));
}

TEST_F(OutgoingStateTest, HandleAosConnectedDoesNothingIfNoWatchdogTimer)
{
    ON_CALL(*pConfigurationManager, GetEpsFallbackWatchdogTime).WillByDefault(Return(-1));

    EXPECT_EQ(
            CallStateName::OUTGOING, pOutgoingState->OnAosStateChanged(MtcAosState::CONNECTED, 0));
}

TEST_F(OutgoingStateTest, HandleAosConnectedInvokesEpsFallbackApis)
{
    ON_CALL(*pConfigurationManager, GetEpsFallbackWatchdogTime).WillByDefault(Return(-1));
    EXPECT_EQ(
            CallStateName::OUTGOING, pOutgoingState->OnAosStateChanged(MtcAosState::CONNECTED, 0));

    ON_CALL(*pConfigurationManager, GetEpsFallbackWatchdogTime).WillByDefault(Return(6000));
    ON_CALL(*pEpsFbTrigger, IsWaitingEpsFallbackForNoResponse).WillByDefault(Return(IMS_FALSE));
    EXPECT_EQ(
            CallStateName::OUTGOING, pOutgoingState->OnAosStateChanged(MtcAosState::CONNECTED, 0));

    ON_CALL(*pEpsFbTrigger, IsWaitingEpsFallbackForNoResponse).WillByDefault(Return(IMS_TRUE));

    EXPECT_CALL(objRedialHelper, Redial).Times(1).WillOnce(Return(IMS_SUCCESS));
    EXPECT_CALL(*pEpsFbTrigger, OnEpsFallbackCompleted);
    EXPECT_EQ(CallStateName::IDLE, pOutgoingState->OnAosStateChanged(MtcAosState::CONNECTED, 0));
}

TEST_F(OutgoingStateTest, OnReceivingMediaDataStartedStopsUdpKeepAliveSender)
{
    MockUdpKeepAliveSender objKeepAliveSender(objCallContext);
    ON_CALL(objCallContext, GetUdpKeepAliveSender).WillByDefault(ReturnRef(objKeepAliveSender));
    ON_CALL(*pConfigurationManager, GetSendUdpKeepAliveIntervalTime).WillByDefault(Return(2000));
    EXPECT_CALL(objKeepAliveSender, Stop);
    pOutgoingState->OnReceivingMediaDataStarted(0, 0);

    ON_CALL(*pConfigurationManager, GetSendUdpKeepAliveIntervalTime).WillByDefault(Return(0));
    EXPECT_CALL(objKeepAliveSender, Stop).Times(0);
    pOutgoingState->OnReceivingMediaDataStarted(0, 0);
}

TEST_F(OutgoingStateTest, OnReceivingNetworkToneStartedInvokesSendProgressing)
{
    ON_CALL(objMediaManager, IsLocalTone()).WillByDefault(Return(IMS_FALSE));
    EXPECT_CALL(objUiNotifier, SendProgressing()).Times(1);
    EXPECT_EQ(CallStateName::OUTGOING, pOutgoingState->OnReceivingNetworkToneStarted());
}

TEST_F(OutgoingStateTest, OnReceivingNetworkToneFailedInvokesSendProgressingWithInactiveAudio)
{
    ON_CALL(objMediaManager, IsLocalTone()).WillByDefault(Return(IMS_TRUE));
    objMediaInfo.eAudioDirection = DIRECTION_INACTIVE;
    EXPECT_CALL(objUiNotifier, SendProgressing()).Times(1);
    EXPECT_EQ(CallStateName::OUTGOING, pOutgoingState->OnReceivingNetworkToneFailed());
}

TEST_F(OutgoingStateTest, QosReservedReturnsOutgoingStateIfPrackTransactionIsNotCompleted)
{
    ON_CALL(objSession, GetPreviousResponse(IMessage::SESSION_PRACK))
            .WillByDefault(Return(nullptr));
    EXPECT_EQ(CallStateName::OUTGOING, pOutgoingState->QosReserved(&objSession, 0));

    MockIMessage objMessage;
    ON_CALL(objSession, GetPreviousResponse(IMessage::SESSION_PRACK))
            .WillByDefault(Return(&objMessage));
    ON_CALL(objMessage, GetStatusCode).WillByDefault(Return(SipStatusCode::SC_INVALID));
    EXPECT_EQ(CallStateName::OUTGOING, pOutgoingState->QosReserved(&objSession, 0));
}

TEST_F(OutgoingStateTest, QosReservedReturnsOutgoingStateIfNotRequiredToSendEarlyUpdate)
{
    MockIMessage objPrackResponseMessage;
    ON_CALL(objSession, GetPreviousResponse(IMessage::SESSION_PRACK))
            .WillByDefault(Return(&objPrackResponseMessage));
    ON_CALL(objPrackResponseMessage, GetStatusCode).WillByDefault(Return(SipStatusCode::SC_200));

    ON_CALL(objPreconditionManager, IsLocalResourceConfirmationRequired(&objSession))
            .WillByDefault(Return(IMS_FALSE));
    EXPECT_EQ(CallStateName::OUTGOING, pOutgoingState->QosReserved(&objSession, 0));
}

TEST_F(OutgoingStateTest, QosReservedReturnsOutgoingStateIfNotAvailableToSendEarlyUpdate)
{
    MockIMessage objPrackResponseMessage;
    ON_CALL(objSession, GetPreviousResponse(IMessage::SESSION_PRACK))
            .WillByDefault(Return(&objPrackResponseMessage));
    ON_CALL(objPrackResponseMessage, GetStatusCode).WillByDefault(Return(SipStatusCode::SC_200));

    ON_CALL(objPreconditionManager, IsLocalResourceConfirmationRequired(&objSession))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objPreconditionManager, IsAvailableToSendLocalResourceConfirmation(&objSession))
            .WillByDefault(Return(IMS_FALSE));
    EXPECT_EQ(CallStateName::OUTGOING, pOutgoingState->QosReserved(&objSession, 0));
}

TEST_F(OutgoingStateTest, QosReservedSendsEarlyUpdate)
{
    MockIMessage objPrackResponseMessage;
    ON_CALL(objSession, GetPreviousResponse(IMessage::SESSION_PRACK))
            .WillByDefault(Return(&objPrackResponseMessage));
    ON_CALL(objPrackResponseMessage, GetStatusCode).WillByDefault(Return(SipStatusCode::SC_200));

    ON_CALL(objPreconditionManager, IsLocalResourceConfirmationRequired(&objSession))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objPreconditionManager, IsAvailableToSendLocalResourceConfirmation(&objSession))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objMediaManager, GetNegotiationState(_))
            .WillByDefault(Return(NegotiationState::STATE_NEGOTIATED));

    EXPECT_CALL(objMtcSession, SendEarlyUpdate(UpdateType::NORMAL))
            .Times(2)
            .WillOnce(Return(IMS_FAILURE))
            .WillOnce(Return(IMS_SUCCESS));

    EXPECT_EQ(CallStateName::TERMINATING, pOutgoingState->QosReserved(&objSession, 0));
    EXPECT_EQ(CallStateName::OUTGOING, pOutgoingState->QosReserved(&objSession, 0));
}

TEST_F(OutgoingStateTest, QosReserveFailedTerminatesCallIfNextActionIsRelease)
{
    EXPECT_CALL(objMtcSession,
            Terminate(_, CallReasonInfo(CODE_LOCAL_CALL_RESOURCE_RESERVATION_FAILED)));

    EXPECT_CALL(objUiNotifier, SendStartFailed(CallReasonInfo(CODE_LOCAL_CALL_CS_RETRY_REQUIRED)));

    EXPECT_EQ(CallStateName::TERMINATING,
            pOutgoingState->QosReserveFailed(&objSession, QosLossPolicy::RELEASE));
}

TEST_F(OutgoingStateTest, QosReserveFailedDoesNothingCallIfNextActionIsNotRelease)
{
    EXPECT_CALL(objMtcSession,
            Terminate(_, CallReasonInfo(CODE_LOCAL_CALL_RESOURCE_RESERVATION_FAILED)))
            .Times(0);

    EXPECT_CALL(objUiNotifier, SendStartFailed(CallReasonInfo(CODE_LOCAL_CALL_CS_RETRY_REQUIRED)))
            .Times(0);

    EXPECT_EQ(CallStateName::OUTGOING,
            pOutgoingState->QosReserveFailed(&objSession, QosLossPolicy::MODIFY));
}

TEST_F(OutgoingStateTest, SessionStartedTerminatesCallIfSdpOaFails)
{
    MockIMessage objMessage;
    ON_CALL(objMessageUtils, GetPreviousResponse(&objSession, IMessage::SESSION_START, -1))
            .WillByDefault(Return(&objMessage));
    SetSdpOaFailure(objMessage);
    ON_CALL(objMessageUtils, GetHeader(_, _, _)).WillByDefault(Return(AString("id")));
    ON_CALL(objMessageUtils, IsHeaderPresent(_, _, _)).WillByDefault(Return(IMS_FALSE));

    EXPECT_CALL(objMtcSession, Terminate(_, CallReasonInfo(CODE_MEDIA_NOT_ACCEPTABLE)));
    EXPECT_CALL(objUiNotifier, SendStartFailed(CallReasonInfo(CODE_MEDIA_NOT_ACCEPTABLE)));

    EXPECT_EQ(CallStateName::TERMINATING, pOutgoingState->SessionStarted(&objSession));
}

TEST_F(OutgoingStateTest, SessionStartedTerminatesCallIfSendingAckFails)
{
    MockIMessage objMessage;
    ON_CALL(objMessageUtils, GetPreviousResponse(&objSession, IMessage::SESSION_START, -1))
            .WillByDefault(Return(&objMessage));
    ON_CALL(objMessageUtils, GetHeader(_, _, _)).WillByDefault(Return(AString("id")));
    ON_CALL(objMessageUtils, IsHeaderPresent(_, _, _)).WillByDefault(Return(IMS_FALSE));
    EXPECT_CALL(objMtcSession, SendAck).Times(1).WillOnce(Return(IMS_FAILURE));

    EXPECT_CALL(objMtcSession, Terminate(_, CallReasonInfo(CODE_REJECT_INTERNAL_ERROR)));
    EXPECT_CALL(objUiNotifier, SendStartFailed(CallReasonInfo(CODE_REJECT_INTERNAL_ERROR)));

    EXPECT_EQ(CallStateName::TERMINATING, pOutgoingState->SessionStarted(&objSession));
}

TEST_F(OutgoingStateTest, SessionStartedInvokesSendStartedToUi)
{
    MockIMessage objMessage;
    ON_CALL(objMessageUtils, GetPreviousResponse(&objSession, IMessage::SESSION_START, -1))
            .WillByDefault(Return(&objMessage));
    ON_CALL(objMessageUtils, GetHeader(_, _, _)).WillByDefault(Return(AString("id")));
    ON_CALL(objMessageUtils, GetHeaderValue(&objMessage, ISipHeader::SESSION_ID, _))
            .WillByDefault(Return(SESSION_ID));
    ON_CALL(objMessageUtils, IsHeaderPresent(_, _, _)).WillByDefault(Return(IMS_FALSE));
    EXPECT_CALL(objMtcSession, SendAck).Times(1).WillOnce(Return(IMS_SUCCESS));
    ON_CALL(objMessage, GetStatusCode).WillByDefault(Return(SipStatusCode::SC_200));

    ON_CALL(objMessageUtils, IsResponseExist(&objSession, SipStatusCode::SC_200))
            .WillByDefault(Return(IMS_TRUE));
    EXPECT_CALL(objMediaManager, Run(&objSession, &objMessage, IMS_FALSE));
    EXPECT_CALL(objUiNotifier, SendStarted);
    EXPECT_CALL(objPreconditionManager, OnCallEstablished(&objSession));

    EXPECT_EQ(CallStateName::ESTABLISHED, pOutgoingState->SessionStarted(&objSession));

    EXPECT_TRUE(pSupplementaryService->Get(SuppType::SESSION_ID));
}

TEST_F(OutgoingStateTest, SessionStartedInvokesStartWatchdogIfSupportedAndSdpAnswerIncluded)
{
    MockIMessage objMessage;
    ON_CALL(objMessageUtils, GetPreviousResponse(&objSession, IMessage::SESSION_START, -1))
            .WillByDefault(Return(&objMessage));
    EXPECT_CALL(objMtcSession, SendAck).Times(1).WillOnce(Return(IMS_SUCCESS));
    ON_CALL(objMessage, GetStatusCode).WillByDefault(Return(SipStatusCode::SC_200));  // useless

    MockISipMessage objSipMessage;
    ON_CALL(objMessage, GetMessage).WillByDefault(Return(&objSipMessage));
    SetSdpOaSuccessWithSdp(objMessage, objSipMessage);  // to cover StartEpsFallbackWatchdogIfNeeded

    ON_CALL(*pConfigurationManager, GetEpsFallbackWatchdogTime).WillByDefault(Return(2000));
    ON_CALL(objService, IsNr).WillByDefault(Return(IMS_TRUE));

    EXPECT_CALL(*pEpsFbTrigger, StartWatchdog);

    EXPECT_EQ(CallStateName::ESTABLISHED, pOutgoingState->SessionStarted(&objSession));
}

TEST_F(OutgoingStateTest, SessionStartedSetsConferenceCallToMediaManagerIfConferenceCall)
{
    objCallInfo.bConference = IMS_TRUE;
    EXPECT_CALL(objMediaManager, SetConferenceCall(IMS_TRUE));

    MockIMessage objMessage;
    ON_CALL(objMessageUtils, GetPreviousResponse(&objSession, IMessage::SESSION_START, -1))
            .WillByDefault(Return(&objMessage));
    ON_CALL(objMessageUtils, GetHeader(_, _, _)).WillByDefault(Return(AString("id")));
    ON_CALL(objMessageUtils, IsHeaderPresent(_, _, _)).WillByDefault(Return(IMS_FALSE));
    EXPECT_CALL(objMtcSession, SendAck).Times(1).WillOnce(Return(IMS_SUCCESS));
    ON_CALL(objMessage, GetStatusCode).WillByDefault(Return(SipStatusCode::SC_200));

    EXPECT_CALL(objUiNotifier, SendStarted);

    EXPECT_EQ(CallStateName::ESTABLISHED, pOutgoingState->SessionStarted(&objSession));
}

TEST_F(OutgoingStateTest, SessionStartFailedKeepsOutgoingStateIfSrvccStartedAndAosDisconnected)
{
    ON_CALL(objService, GetSrvccState).WillByDefault(Return(SrvccState::STARTED));
    ON_CALL(objAosConnector, IsImsConnected).WillByDefault(Return(IMS_FALSE));

    EXPECT_EQ(CallStateName::OUTGOING, pOutgoingState->SessionStartFailed(&objSession));
}

TEST_F(OutgoingStateTest, SessionStartFailedTriggersEpsFallbackByCallReason)
{
    ON_CALL(objService, GetSrvccState).WillByDefault(Return(SrvccState::IDLE));

    MockIMessage objMessage;
    ON_CALL(objMessageUtils, GetPreviousResponse(&objSession, IMessage::SESSION_START, -1))
            .WillByDefault(Return(&objMessage));
    SetUpStartErrorHandler(objMessage, SipStatusCode::SC_INVALID, IMS_FALSE,
            CarrierConfig::ImsVoice::MO_CALL_REQUEST_TIMEOUT_POLICY_REDIAL_BY_NETWORK_CONTEXT,
            IMS_TRUE, IMS_FALSE);

    EXPECT_CALL(
            *pEpsFbTrigger, TriggerEpsFallback(EpsFallbackReason::NO_NETWORK_RESPONSE, IMS_TRUE));

    EXPECT_EQ(CallStateName::OUTGOING, pOutgoingState->SessionStartFailed(&objSession));
}

TEST_F(OutgoingStateTest, SessionStartFailedInvokesRedialByCallReason)
{
    ON_CALL(objService, GetSrvccState).WillByDefault(Return(SrvccState::IDLE));

    MockIMessage objMessage;
    ON_CALL(objMessageUtils, GetPreviousResponse(&objSession, IMessage::SESSION_START, -1))
            .WillByDefault(Return(&objMessage));
    SetUpStartErrorHandler(objMessage, SipStatusCode::SC_INVALID, IMS_FALSE,
            CarrierConfig::ImsVoice::MO_CALL_REQUEST_TIMEOUT_POLICY_SILENT_REDIAL, IMS_FALSE,
            IMS_FALSE);
    EXPECT_CALL(objRedialHelper, Redial).Times(1).WillOnce(Return(IMS_SUCCESS));

    EXPECT_EQ(CallStateName::IDLE, pOutgoingState->SessionStartFailed(&objSession));
}

TEST_F(OutgoingStateTest, SessionStartFailedSetsNetworkResponseTimoutReasonIfSilentRedialFails)
{
    ON_CALL(objService, GetSrvccState).WillByDefault(Return(SrvccState::IDLE));

    MockIMessage objMessage;
    ON_CALL(objMessageUtils, GetPreviousResponse(&objSession, IMessage::SESSION_START, -1))
            .WillByDefault(Return(&objMessage));
    SetUpStartErrorHandler(objMessage, SipStatusCode::SC_INVALID, IMS_FALSE,
            CarrierConfig::ImsVoice::MO_CALL_REQUEST_TIMEOUT_POLICY_SILENT_REDIAL, IMS_FALSE,
            IMS_FALSE);

    ON_CALL(objRedialHelper, Redial).WillByDefault(Return(IMS_FAILURE));

    EXPECT_CALL(objUiNotifier,
            SendStartFailed(CallReasonInfo(CODE_NETWORK_RESP_TIMEOUT, EXTRA_CODE_METHOD_INVITE)));
    EXPECT_EQ(CallStateName::TERMINATING, pOutgoingState->SessionStartFailed(&objSession));
}

TEST_F(OutgoingStateTest, SessionStartFailedSetsSipNotAcceptableReasonIfSilentRedialFails)
{
    ON_CALL(objService, GetSrvccState).WillByDefault(Return(SrvccState::IDLE));

    MockIMessage objMessage;
    ON_CALL(objMessageUtils, GetPreviousResponse(&objSession, IMessage::SESSION_START, -1))
            .WillByDefault(Return(&objMessage));
    SetUpStartErrorHandler(objMessage, SipStatusCode::SC_488, IMS_FALSE, 0, IMS_FALSE, IMS_FALSE);
    ON_CALL(objMessageUtils, HasSdp(&objMessage)).WillByDefault(Return(IMS_TRUE));

    ON_CALL(objMediaManager, GetSupportedMediaTypesFromSdp(&objSession))
            .WillByDefault(Return(MEDIATYPE_AUDIO));

    ON_CALL(objRedialHelper, Redial).WillByDefault(Return(IMS_FAILURE));

    EXPECT_CALL(objUiNotifier,
            SendStartFailed(CallReasonInfo(CODE_SIP_NOT_ACCEPTABLE, SipStatusCode::SC_488)));
    EXPECT_EQ(CallStateName::TERMINATING, pOutgoingState->SessionStartFailed(&objSession));
}

TEST_F(OutgoingStateTest, SessionStartFailedSetsSipRedirectedReasonIfSilentRedialFails)
{
    ON_CALL(objService, GetSrvccState).WillByDefault(Return(SrvccState::IDLE));

    MockIMessage objMessage;
    ON_CALL(objMessageUtils, GetPreviousResponse(&objSession, IMessage::SESSION_START, -1))
            .WillByDefault(Return(&objMessage));
    SetUpStartErrorHandler(objMessage, SipStatusCode::SC_301, IMS_FALSE, 0, IMS_FALSE, IMS_FALSE);
    AString strContactToRedirect("sip:contactToRedirect");
    ON_CALL(objMessageUtils, GetHeaderValue(&objMessage, ISipHeader::CONTACT_NORMAL, _))
            .WillByDefault(Return(strContactToRedirect));

    ON_CALL(objRedialHelper, Redial).WillByDefault(Return(IMS_FAILURE));

    EXPECT_CALL(objUiNotifier,
            SendStartFailed(CallReasonInfo(CODE_SIP_REDIRECTED, SipStatusCode::SC_301)));
    EXPECT_EQ(CallStateName::TERMINATING, pOutgoingState->SessionStartFailed(&objSession));
}

TEST_F(OutgoingStateTest, SessionStartFailedInvokesStartFailed)
{
    ON_CALL(objService, GetSrvccState).WillByDefault(Return(SrvccState::IDLE));

    MockIMessage objMessage;
    ON_CALL(objMessageUtils, GetPreviousResponse(&objSession, IMessage::SESSION_START, -1))
            .WillByDefault(Return(&objMessage));
    SetUpStartErrorHandler(objMessage, SipStatusCode::SC_600, IMS_TRUE, 0, IMS_FALSE, IMS_FALSE);

    EXPECT_CALL(objUiNotifier,
            SendStartFailed(CallReasonInfo(
                    CODE_LOCAL_CALL_CS_RETRY_REQUIRED, EXTRA_CODE_CALL_RETRY_SILENT_REDIAL)));
    EXPECT_EQ(CallStateName::TERMINATING, pOutgoingState->SessionStartFailed(&objSession));
}

TEST_F(OutgoingStateTest,
        SessionStartFailedHandlesCountrySpecificServiceUrnIfReasonIsAlternateEccByConfig)
{
    ON_CALL(objService, GetSrvccState).WillByDefault(Return(SrvccState::IDLE));

    MockIMessage objMessage;
    ON_CALL(objMessageUtils, GetPreviousResponse(&objSession, IMessage::SESSION_START, -1))
            .WillByDefault(Return(&objMessage));
    ON_CALL(objMessageUtils, GetSosTypeFromServiceUrn(_, ISipHeader::CONTACT_NORMAL, _))
            .WillByDefault(Return(EXTRA_CODE_EMERGENCYSERVICE_COUNTRY_SPECIFIC));
    SetUpStartErrorHandler(objMessage, SipStatusCode::SC_380, IMS_FALSE, 0, IMS_FALSE, IMS_FALSE);
    ON_CALL(*pConfigurationManager,
            IsEmergencyRetryWithoutChecking380ContentForNonUeDetectableEmergencyCall)
            .WillByDefault(Return(IMS_TRUE));

    EXPECT_CALL(objUiNotifier,
            SendStartFailed(CallReasonInfo(CODE_SIP_ALTERNATE_EMERGENCY_CALL,
                    EXTRA_CODE_EMERGENCYSERVICE_COUNTRY_SPECIFIC)));

    ON_CALL(objSession, GetPreviousResponse(IMessage::SESSION_START))
            .WillByDefault(Return(&objMessage));
    ON_CALL(objMessageUtils, IsHeaderPresent(&objMessage, ISipHeader::CONTACT_NORMAL, _))
            .WillByDefault(Return(IMS_TRUE));
    AString strUrn("<urn:service:sos.country-specific.ANY>");
    ON_CALL(objMessageUtils, GetHeader(&objMessage, ISipHeader::CONTACT_NORMAL, _))
            .WillByDefault(Return(strUrn));
    AString strNumber("anyNumber");
    ON_CALL(objMessageUtils, GetUserPart(&objMessage, ISipHeader::TO, _))
            .WillByDefault(Return(strNumber));
    MockIMtcDialingPlan objDialingPlan;
    ON_CALL(objCallContext, GetDialingPlan).WillByDefault(ReturnRef(objDialingPlan));
    EXPECT_CALL(objDialingPlan, OnCountrySpecificServiceUrnReceived(strNumber, strUrn));

    EXPECT_EQ(CallStateName::TERMINATING, pOutgoingState->SessionStartFailed(&objSession));
}

TEST_F(OutgoingStateTest,
        SessionStartFailedHandlesCountrySpecificServiceUrnIfReasonIsAlternateEccByContext)
{
    /*
        ON_CALL(objService, GetSrvccState).WillByDefault(Return(SrvccState::IDLE));

        MockIMessage objMessage;
        ON_CALL(objMessageUtils, GetPreviousResponse(&objSession, IMessage::SESSION_START, -1))
                .WillByDefault(Return(&objMessage));
        ON_CALL(objMessageUtils, GetSosTypeFromServiceUrn(_, ISipHeader::CONTACT_NORMAL, _))
                .WillByDefault(Return(EXTRA_CODE_EMERGENCYSERVICE_COUNTRY_SPECIFIC));
        SetUpStartErrorHandler(objMessage, SipStatusCode::SC_380, IMS_FALSE, 0, IMS_FALSE,
       IMS_FALSE); ON_CALL(*pConfigurationManager,
                IsEmergencyRetryWithoutChecking380ContentForNonUeDetectableEmergencyCall)
                .WillByDefault(Return(IMS_FALSE));

        // TODO: mock Ims3gpp for more coverage in StartErrorHandler
        Ims3gpp objIms3gpp;
        ON_CALL(objMessageUtils, GetIms3gppFromBody(_, _))
                .WillByDefault(ReturnRef(objIms3gpp));

        AString strSupported("no p a t h");
        ON_CALL(objAosConnector, GetSupportedHeaderValue).WillByDefault(Return(strSupported));
        EXPECT_CALL(objUiNotifier,
                SendStartFailed(CallReasonInfo(
                        CODE_SIP_ALTERNATE_EMERGENCY_CALL,
                        EXTRA_CODE_EMERGENCYSERVICE_COUNTRY_SPECIFIC)));

        ON_CALL(objSession, GetPreviousResponse(IMessage::SESSION_START))
                .WillByDefault(Return(&objMessage));
        ON_CALL(objMessageUtils, IsHeaderPresent(&objMessage, ISipHeader::CONTACT_NORMAL, _))
                .WillByDefault(Return(IMS_TRUE));
        AString strUrn("urn:service:sos.country-specific.ANY");
        ON_CALL(objMessageUtils, GetHeader(&objMessage, ISipHeader::CONTACT_NORMAL, _))
                .WillByDefault(Return(strUrn));
        AString strNumber("anyNumber");
        ON_CALL(objMessageUtils, GetUserPart(&objMessage, ISipHeader::TO, _))
                .WillByDefault(Return(strNumber));
        MockIMtcDialingPlan objDialingPlan;
        ON_CALL(objCallContext, GetDialingPlan)
                .WillByDefault(ReturnRef(objDialingPlan));
        EXPECT_CALL(objDialingPlan, OnCountrySpecificServiceUrnReceived(strNumber, strUrn));

        EXPECT_EQ(CallStateName::TERMINATING, pOutgoingState->SessionStartFailed(&objSession));
    */
}

TEST_F(OutgoingStateTest, SessionStartFailedIfWaitingForSilentEmergencyRedial)
{
    objCallInfo.bEmergency = IMS_TRUE;
    ON_CALL(objService, GetSrvccState()).WillByDefault(Return(SrvccState::IDLE));

    MockIMessage objMessage;
    ON_CALL(objMessageUtils, GetPreviousResponse(&objSession, IMessage::SESSION_START, -1))
            .WillByDefault(Return(&objMessage));

    ON_CALL(*pConfigurationManager, IsRetryEmergencyCallOverEmergencyPdnWithNextPcscf())
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objService, IsEmergency()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(objMessage, GetStatusCode()).WillByDefault(Return(SipStatusCode::SC_400));
    ON_CALL(objMessageUtils, GetNumberOfPreviousResponses(&objSession, IMessage::SESSION_START))
            .WillByDefault(Return(1));

    EXPECT_CALL(objAosConnector, Control).Times(1);
    EXPECT_EQ(CallStateName::OUTGOING, pOutgoingState->SessionStartFailed(&objSession));

    EXPECT_CALL(objRedialHelper, Redial(_)).Times(1).WillOnce(Return(IMS_SUCCESS));
    EXPECT_EQ(CallStateName::IDLE, pOutgoingState->OnAosStateChanged(MtcAosState::CONNECTED, 0));
}

TEST_F(OutgoingStateTest, SessionStartFailedIfWaitingForSilentNormalRedial)
{
    ON_CALL(objService, GetSrvccState()).WillByDefault(Return(SrvccState::IDLE));
    ON_CALL(objTimer, IsActive(MtcCallState::TimerType::TIMER_MO_100_WAIT))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objTimer, IsActive(MtcCallState::TimerType::TIMER_MO_18X_WAIT))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(*pConfigurationManager,
            IsRegistrationDisconnectReasonToIgnore(ImsAosReason::REG_NEW_REQUIRED))
            .WillByDefault(Return(IMS_TRUE));

    MockIMessage objMessage;
    ON_CALL(objMessageUtils, GetPreviousResponse(&objSession, IMessage::SESSION_START, -1))
            .WillByDefault(Return(&objMessage));

    ON_CALL(objMessage, GetStatusCode()).WillByDefault(Return(SipStatusCode::SC_503));
    ImsList<IMtcCall*> objCalls;
    ON_CALL(objCallManager, GetCallsByState(_)).WillByDefault(Return(objCalls));

    EXPECT_CALL(objTimer, Stop(MtcCallState::TimerType::TIMER_MO_100_WAIT));
    EXPECT_CALL(objTimer, Stop(MtcCallState::TimerType::TIMER_MO_18X_WAIT));
    EXPECT_CALL(objAosConnector, RegisterWithNextPcscf(0)).Times(1);
    EXPECT_EQ(CallStateName::OUTGOING, pOutgoingState->SessionStartFailed(&objSession));
    EXPECT_CALL(objMessageUtils, GetPreviousResponse(&objSession, IMessage::SESSION_START, -1))
            .Times(0);
    EXPECT_EQ(CallStateName::OUTGOING, pOutgoingState->SessionStartFailed(&objSession));

    EXPECT_EQ(CallStateName::OUTGOING,
            pOutgoingState->OnAosStateChanged(
                    MtcAosState::DISCONNECTED, ImsAosReason::REG_NEW_REQUIRED));

    EXPECT_CALL(objRedialHelper, Redial(_)).Times(1).WillOnce(Return(IMS_SUCCESS));
    EXPECT_EQ(CallStateName::IDLE, pOutgoingState->OnAosStateChanged(MtcAosState::CONNECTED, 0));
}

TEST_F(OutgoingStateTest, SessionTerminatedNotifiesStartFailed)
{
    ON_CALL(objSession, GetTerminationReason)
            .WillByDefault(Return(ISession::TERMINATION_REASON_REFRESH_408));

    EXPECT_CALL(objUiNotifier,
            SendStartFailed(CallReasonInfo(
                    CODE_SIP_REQUEST_TIMEOUT, ISession::TERMINATION_REASON_REFRESH_408)));
    EXPECT_EQ(CallStateName::TERMINATING, pOutgoingState->SessionTerminated(&objSession));
}

TEST_F(OutgoingStateTest, SessionEarlyMediaUpdatedTerminatesCallIfSdpOaFails)
{
    MockIMessage objMessage;
    ON_CALL(objMessageUtils, GetPreviousResponse(&objSession, IMessage::SESSION_EARLY_UPDATE, -1))
            .WillByDefault(Return(&objMessage));
    SetSdpOaFailure(objMessage);

    EXPECT_CALL(objMtcSession, Terminate(_, CallReasonInfo(CODE_MEDIA_NOT_ACCEPTABLE)));
    EXPECT_CALL(objUiNotifier, SendStartFailed(CallReasonInfo(CODE_MEDIA_NOT_ACCEPTABLE)));

    EXPECT_EQ(CallStateName::TERMINATING, pOutgoingState->SessionEarlyMediaUpdated(&objSession));
}

TEST_F(OutgoingStateTest, SessionEarlyMediaUpdatedInvokesSendProgressing)
{
    MockIMessage objMessage;
    ON_CALL(objMessageUtils, GetPreviousResponse(&objSession, IMessage::SESSION_EARLY_UPDATE, -1))
            .WillByDefault(Return(&objMessage));

    ON_CALL(objMessageUtils, IsResponseExist(&objSession, SipStatusCode::SC_200))
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMediaManager, IsLocalTone).WillByDefault(Return(IMS_TRUE));
    MediaInfo objMediaInfo;
    objMediaInfo.eAudioDirection = DIRECTION_INACTIVE;
    EXPECT_CALL(objMediaManager, Run(&objSession, &objMessage, IMS_TRUE));
    EXPECT_CALL(objUiNotifier, SendProgressing());

    EXPECT_EQ(CallStateName::OUTGOING, pOutgoingState->SessionEarlyMediaUpdated(&objSession));
}

TEST_F(OutgoingStateTest, SessionEarlyMediaUpdateFailedReturnsTerminating)
{
    MockIMessage objMessage;
    ON_CALL(objMessageUtils, GetPreviousResponse(&objSession, IMessage::SESSION_EARLY_UPDATE, -1))
            .WillByDefault(Return(&objMessage));
    ON_CALL(objMessage, GetStatusCode).WillByDefault(Return(SipStatusCode::SC_INVALID));

    EXPECT_CALL(objMtcSession,
            Terminate(_, CallReasonInfo(CODE_NETWORK_RESP_TIMEOUT, EXTRA_CODE_METHOD_UPDATE)));
    EXPECT_CALL(objUiNotifier,
            SendStartFailed(CallReasonInfo(CODE_NETWORK_RESP_TIMEOUT, EXTRA_CODE_METHOD_UPDATE)));

    EXPECT_EQ(
            CallStateName::TERMINATING, pOutgoingState->SessionEarlyMediaUpdateFailed(&objSession));
}

TEST_F(OutgoingStateTest, SessionEarlyMediaUpdateFailedWith491StartsGlareCondition)
{
    MockIMessage objMessage;
    ON_CALL(objMessage, GetStatusCode).WillByDefault(Return(SipStatusCode::SC_491));
    ON_CALL(objMessageUtils, GetPreviousResponse(&objSession, IMessage::SESSION_EARLY_UPDATE, _))
            .WillByDefault(Return(&objMessage));
    ON_CALL(objCallContext, GetCallInfo).WillByDefault(ReturnRef(objCallInfo));

    EXPECT_CALL(objMtcSession, Terminate(_, _)).Times(0);
    EXPECT_CALL(objUiNotifier, SendStartFailed(_)).Times(0);
    EXPECT_CALL(objMediaManager, FinalizeSdp(&objSession));
    EXPECT_CALL(objTimer, Start(MtcCallState::TimerType::TIMER_GLARE_CONDITION, _));

    EXPECT_EQ(CallStateName::OUTGOING, pOutgoingState->SessionEarlyMediaUpdateFailed(&objSession));
}

TEST_F(OutgoingStateTest, SessionEarlyMediaUpdateFailedWith503WaitsRedial)
{
    MockIMessage objMessage;
    ON_CALL(objMessage, GetStatusCode).WillByDefault(Return(SipStatusCode::SC_503));
    ON_CALL(objMessageUtils, GetPreviousResponse(&objSession, IMessage::SESSION_EARLY_UPDATE, _))
            .WillByDefault(Return(&objMessage));
    ImsList<IMtcCall*> objCalls;
    ON_CALL(objCallManager, GetCallsByState(_)).WillByDefault(Return(objCalls));
    EXPECT_CALL(objAosConnector, RegisterWithNextPcscf(0)).Times(1);

    EXPECT_CALL(objMtcSession, Terminate(_, _)).Times(0);
    EXPECT_CALL(objUiNotifier, SendStartFailed(_)).Times(0);
    EXPECT_CALL(objMediaManager, FinalizeSdp(&objSession)).Times(0);
    EXPECT_CALL(objTimer, Start(MtcCallState::TimerType::TIMER_GLARE_CONDITION, _)).Times(0);

    EXPECT_EQ(CallStateName::OUTGOING, pOutgoingState->SessionEarlyMediaUpdateFailed(&objSession));

    EXPECT_EQ(CallStateName::OUTGOING,
            pOutgoingState->OnAosStateChanged(
                    MtcAosState::DISCONNECTED, ImsAosReason::REG_NEW_REQUIRED));

    EXPECT_CALL(objRedialHelper, Redial(_)).Times(1).WillOnce(Return(IMS_SUCCESS));
    EXPECT_EQ(CallStateName::IDLE, pOutgoingState->OnAosStateChanged(MtcAosState::CONNECTED, 0));
}

TEST_F(OutgoingStateTest, SessionEarlyMediaUpdateFailedWith503InvokesRedial)
{
    MockIMessage objMessage;
    ON_CALL(objMessage, GetStatusCode).WillByDefault(Return(SipStatusCode::SC_503));
    ON_CALL(objMessageUtils, GetPreviousResponse(&objSession, IMessage::SESSION_EARLY_UPDATE, _))
            .WillByDefault(Return(&objMessage));
    ImsList<IMtcCall*> objCalls;
    ON_CALL(objCallManager, GetCallsByState(_)).WillByDefault(Return(objCalls));

    EXPECT_CALL(objMtcSession, Terminate(_, _)).Times(0);
    EXPECT_CALL(objUiNotifier, SendStartFailed(_)).Times(0);
    EXPECT_CALL(objMediaManager, FinalizeSdp(&objSession)).Times(0);
    EXPECT_CALL(objTimer, Start(MtcCallState::TimerType::TIMER_GLARE_CONDITION, _)).Times(0);

    IMS_SINT32 nAnyRetryAfter = 10;
    ON_CALL(objMessageUtils, GetHeaderValueInt(&objMessage, ISipHeader::RETRY_AFTER_ANY, _))
            .WillByDefault(Return(nAnyRetryAfter));
    ON_CALL(pConfigService->GetMockCarrierConfig(),
            GetInt(CarrierConfig::Ims::KEY_SIP_TIMER_F_MILLIS_INT, _))
            .WillByDefault(Return((nAnyRetryAfter + 1) * 1000));
    Engine::GetConfiguration()->RefreshConfigs(objCallContext.GetSlotId());
    ON_CALL(objService, IsEpsCombinedAttach).WillByDefault(Return(IMS_FALSE));

    EXPECT_CALL(objRedialHelper, Redial(_)).Times(1).WillOnce(Return(IMS_SUCCESS));
    EXPECT_EQ(CallStateName::IDLE, pOutgoingState->SessionEarlyMediaUpdateFailed(&objSession));
}

TEST_F(OutgoingStateTest, SessionEarlyMediaUpdateReceivedRejectsRequestIfSdpOaFails)
{
    MockIMessage objMessage;
    ON_CALL(objSession, GetPreviousRequest(IMessage::SESSION_EARLY_UPDATE))
            .WillByDefault(Return(&objMessage));
    SetSdpOaFailure(objMessage);

    EXPECT_CALL(objMtcSession, RespondToEarlyUpdate(SipStatusCode::SC_488))
            .WillOnce(Return(IMS_SUCCESS));

    EXPECT_EQ(
            CallStateName::OUTGOING, pOutgoingState->SessionEarlyMediaUpdateReceived(&objSession));
}

TEST_F(OutgoingStateTest, SessionEarlyMediaUpdateReceivedTerminatesCallIfSending488Fails)
{
    MockIMessage objMessage;
    ON_CALL(objSession, GetPreviousRequest(IMessage::SESSION_EARLY_UPDATE))
            .WillByDefault(Return(&objMessage));
    SetSdpOaFailure(objMessage);

    EXPECT_CALL(objMtcSession, RespondToEarlyUpdate(SipStatusCode::SC_488))
            .WillOnce(Return(IMS_FAILURE));

    EXPECT_CALL(objMtcSession, Terminate(_, CallReasonInfo(CODE_MEDIA_NOT_ACCEPTABLE)));
    EXPECT_CALL(objUiNotifier, SendStartFailed(CallReasonInfo(CODE_MEDIA_NOT_ACCEPTABLE)));

    EXPECT_EQ(CallStateName::TERMINATING,
            pOutgoingState->SessionEarlyMediaUpdateReceived(&objSession));
}

TEST_F(OutgoingStateTest, SessionEarlyMediaUpdateReceivedTerminatesCallIfSending200Fails)
{
    MockIMessage objMessage;
    ON_CALL(objSession, GetPreviousRequest(IMessage::SESSION_EARLY_UPDATE))
            .WillByDefault(Return(&objMessage));

    EXPECT_CALL(objMtcSession, RespondToEarlyUpdate(SipStatusCode::SC_200))
            .WillOnce(Return(IMS_FAILURE));

    EXPECT_CALL(objMtcSession, Terminate(_, CallReasonInfo(CODE_REJECT_INTERNAL_ERROR)));
    EXPECT_CALL(objUiNotifier, SendStartFailed(CallReasonInfo(CODE_REJECT_INTERNAL_ERROR)));

    EXPECT_EQ(CallStateName::TERMINATING,
            pOutgoingState->SessionEarlyMediaUpdateReceived(&objSession));
}

TEST_F(OutgoingStateTest, SessionEarlyMediaUpdateReceivedInvokesSendProgressing)
{
    MockIMessage objMessage;
    ON_CALL(objSession, GetPreviousRequest(IMessage::SESSION_EARLY_UPDATE))
            .WillByDefault(Return(&objMessage));

    MockISipMessage objSipMessage;
    ON_CALL(objMessage, GetMessage).WillByDefault(Return(&objSipMessage));
    SetSdpOaSuccessWithSdp(objMessage, objSipMessage);  // to cover OnMessageReceived()
    EXPECT_CALL(objPreconditionManager, OnMessageReceived(&objSession, &objMessage));

    EXPECT_CALL(objMtcSession, RespondToEarlyUpdate(SipStatusCode::SC_200))
            .WillOnce(Return(IMS_SUCCESS));

    ON_CALL(objMessageUtils, IsResponseExist(&objSession, SipStatusCode::SC_200))
            .WillByDefault(Return(IMS_FALSE));
    EXPECT_CALL(objMediaManager, Run(&objSession, &objMessage, IMS_TRUE));
    EXPECT_CALL(objUiNotifier, SendProgressing());

    EXPECT_EQ(
            CallStateName::OUTGOING, pOutgoingState->SessionEarlyMediaUpdateReceived(&objSession));
}

TEST_F(OutgoingStateTest, SessionForkedResponseReceivedDoesNothingIfSessionIsNull)
{
    EXPECT_CALL(*pSessionInterfaceHolder, AddISession(_)).Times(0);
    EXPECT_EQ(CallStateName::OUTGOING,
            pOutgoingState->SessionForkedResponseReceived(IMS_NULL, IMS_NULL));

    EXPECT_EQ(CallStateName::OUTGOING,
            pOutgoingState->SessionForkedResponseReceived(&objSession, IMS_NULL));
}

TEST_F(OutgoingStateTest, SessionForkedResponseReceivedAddsISession)
{
    ON_CALL(*pConfigurationManager, IsMaintainMultipleEarlySessionsByForking)
            .WillByDefault(Return(IMS_TRUE));
    EXPECT_CALL(objMtcSession,
            Terminate(IMS_TRUE, CallReasonInfo(CODE_INTERNAL_EARLYDIALOG_FORKED_TERMINATED)))
            .Times(0);
    EXPECT_CALL(objCallContext, RemoveSession(&objSession)).Times(0);

    MockISession objForkedSession;
    EXPECT_CALL(*pSessionInterfaceHolder, AddISession(&objForkedSession));
    EXPECT_EQ(CallStateName::OUTGOING,
            pOutgoingState->SessionForkedResponseReceived(&objSession, &objForkedSession));
}

TEST_F(OutgoingStateTest, SessionForkedResponseReceivedTerminatesOriginalSessionByConfig)
{
    ON_CALL(*pConfigurationManager, IsMaintainMultipleEarlySessionsByForking)
            .WillByDefault(Return(IMS_FALSE));
    EXPECT_CALL(objMtcSession,
            Terminate(IMS_TRUE, CallReasonInfo(CODE_INTERNAL_EARLYDIALOG_FORKED_TERMINATED)))
            .Times(1);
    EXPECT_CALL(objCallContext, RemoveSession(&objSession)).Times(1);

    MockISession objForkedSession;
    EXPECT_EQ(CallStateName::OUTGOING,
            pOutgoingState->SessionForkedResponseReceived(&objSession, &objForkedSession));
}

TEST_F(OutgoingStateTest, SessionForkedResponseReceivedDoesNotTerminateOriginalSessionIfNotFound)
{
    ON_CALL(*pConfigurationManager, IsMaintainMultipleEarlySessionsByForking)
            .WillByDefault(Return(IMS_FALSE));

    ON_CALL(objCallContext, GetSession(_)).WillByDefault(Return(nullptr));

    EXPECT_CALL(objMtcSession,
            Terminate(IMS_TRUE, CallReasonInfo(CODE_INTERNAL_EARLYDIALOG_FORKED_TERMINATED)))
            .Times(0);
    EXPECT_CALL(objCallContext, RemoveSession(&objSession)).Times(0);

    MockISession objForkedSession;
    EXPECT_EQ(CallStateName::OUTGOING,
            pOutgoingState->SessionForkedResponseReceived(&objSession, &objForkedSession));
}

TEST_F(OutgoingStateTest, SessionPrackDeliveredReturnsOutgoingAndDoesNothingIfNoMessageFound)
{
    ON_CALL(objSession, GetPreviousResponse(IMessage::SESSION_PRACK))
            .WillByDefault(Return(nullptr));
    EXPECT_CALL(objMtcSession, HandleResponse(_, _)).Times(0);
    EXPECT_EQ(CallStateName::OUTGOING, pOutgoingState->SessionPrackDelivered(&objSession));
}

TEST_F(OutgoingStateTest, SessionPrackDeliveredReturnsOutgoingIfNoPreconditionSupported)
{
    MockIMessage objMessage;
    ON_CALL(objSession, GetPreviousResponse(IMessage::SESSION_PRACK))
            .WillByDefault(Return(&objMessage));

    ON_CALL(objPreconditionManager, IsLocalResourceConfirmationRequired(&objSession))
            .WillByDefault(Return(IMS_FALSE));
    EXPECT_CALL(objMtcSession, HandleResponse(_, _)).Times(1);
    EXPECT_EQ(CallStateName::OUTGOING, pOutgoingState->SessionPrackDelivered(&objSession));
}

TEST_F(OutgoingStateTest, SessionPrackDeliveredReturnsOutgoingIfNotAvailableToSendEarlyUpdate)
{
    MockIMessage objMessage;
    ON_CALL(objSession, GetPreviousResponse(IMessage::SESSION_PRACK))
            .WillByDefault(Return(&objMessage));

    ON_CALL(objPreconditionManager, IsLocalResourceConfirmationRequired(&objSession))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objPreconditionManager, IsAvailableToSendLocalResourceConfirmation(&objSession))
            .WillByDefault(Return(IMS_FALSE));
    EXPECT_CALL(objMessageUtils, GetResponseStatusCode(_, _, _)).Times(0);
    EXPECT_EQ(CallStateName::OUTGOING, pOutgoingState->SessionPrackDelivered(&objSession));
}

TEST_F(OutgoingStateTest, SessionPrackDeliveredReturnsOutgoingIfCodeIs183ButNotNegotiated)
{
    MockIMessage objMessage;
    ON_CALL(objSession, GetPreviousResponse(IMessage::SESSION_PRACK))
            .WillByDefault(Return(&objMessage));

    ON_CALL(objPreconditionManager, IsLocalResourceConfirmationRequired(&objSession))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objPreconditionManager, IsAvailableToSendLocalResourceConfirmation(&objSession))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objMessageUtils, GetResponseStatusCode(&objSession, IMessage::SESSION_START, -1))
            .WillByDefault(Return(SipStatusCode::SC_183));
    ON_CALL(objMediaManager, GetNegotiationState(&objSession))
            .WillByDefault(Return(NegotiationState::STATE_OFFER_SENT));

    EXPECT_EQ(CallStateName::OUTGOING, pOutgoingState->SessionPrackDelivered(&objSession));
}

TEST_F(OutgoingStateTest, SessionPrackDeliveredSendsEarlyUpdateIfAvailable)
{
    MockIMessage objMessage;
    ON_CALL(objSession, GetPreviousResponse(IMessage::SESSION_PRACK))
            .WillByDefault(Return(&objMessage));

    ON_CALL(objPreconditionManager, IsLocalResourceConfirmationRequired(&objSession))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objPreconditionManager, IsAvailableToSendLocalResourceConfirmation(&objSession))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objMessageUtils, GetResponseStatusCode(&objSession, IMessage::SESSION_START, -1))
            .WillByDefault(Return(SipStatusCode::SC_183));
    ON_CALL(objMediaManager, GetNegotiationState(&objSession))
            .WillByDefault(Return(NegotiationState::STATE_NEGOTIATED));

    EXPECT_CALL(objMtcSession, SendEarlyUpdate(UpdateType::NORMAL)).WillOnce(Return(IMS_SUCCESS));

    EXPECT_EQ(CallStateName::OUTGOING, pOutgoingState->SessionPrackDelivered(&objSession));
}

TEST_F(OutgoingStateTest, SessionPrackDeliveredTerminatesCallIfSendingUpdateFails)
{
    MockIMessage objMessage;
    ON_CALL(objSession, GetPreviousResponse(IMessage::SESSION_PRACK))
            .WillByDefault(Return(&objMessage));

    ON_CALL(objPreconditionManager, IsLocalResourceConfirmationRequired(&objSession))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objPreconditionManager, IsAvailableToSendLocalResourceConfirmation(&objSession))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objMessageUtils, GetResponseStatusCode(&objSession, IMessage::SESSION_START, -1))
            .WillByDefault(Return(SipStatusCode::SC_183));
    ON_CALL(objMediaManager, GetNegotiationState(&objSession))
            .WillByDefault(Return(NegotiationState::STATE_NEGOTIATED));

    EXPECT_CALL(objMtcSession, SendEarlyUpdate(UpdateType::NORMAL)).WillOnce(Return(IMS_FAILURE));

    EXPECT_CALL(objMtcSession, Terminate(_, CallReasonInfo(CODE_REJECT_INTERNAL_ERROR)));
    EXPECT_CALL(objUiNotifier, SendStartFailed(CallReasonInfo(CODE_REJECT_INTERNAL_ERROR)));

    EXPECT_EQ(CallStateName::TERMINATING, pOutgoingState->SessionPrackDelivered(&objSession));
}

TEST_F(OutgoingStateTest, SessionPrackDeliveredReturnsOutgoingIf200OkIsAlreadyReceived)
{
    MockIMessage objMessage;
    ON_CALL(objSession, GetPreviousResponse(IMessage::SESSION_PRACK))
            .WillByDefault(Return(&objMessage));

    ON_CALL(objPreconditionManager, IsLocalResourceConfirmationRequired(&objSession))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objPreconditionManager, IsAvailableToSendLocalResourceConfirmation(&objSession))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objMessageUtils, GetResponseStatusCode(&objSession, IMessage::SESSION_START, -1))
            .WillByDefault(Return(SipStatusCode::SC_200));

    EXPECT_CALL(objMtcSession, SendEarlyUpdate(UpdateType::NORMAL)).Times(0);

    EXPECT_EQ(CallStateName::OUTGOING, pOutgoingState->SessionPrackDelivered(&objSession));
}

TEST_F(OutgoingStateTest, SessionPrackDeliveryFailedDoesNothingIfNeedToIgnoreByConfig)
{
    ON_CALL(*pConfigurationManager, IsIgnorePrackDeliveryFailure).WillByDefault(Return(IMS_TRUE));

    EXPECT_EQ(CallStateName::OUTGOING, pOutgoingState->SessionPrackDeliveryFailed(&objSession));
}

TEST_F(OutgoingStateTest, SessionPrackDeliveryFailedByErrorResponseTerminatesCall)
{
    ON_CALL(*pConfigurationManager, IsIgnorePrackDeliveryFailure).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMessageUtils, GetResponseStatusCode(&objSession, IMessage::SESSION_PRACK, -1))
            .WillByDefault(Return(SipStatusCode::SC_488));

    EXPECT_CALL(objMtcSession,
            Terminate(_, CallReasonInfo(CODE_SIP_METHOD_NOT_ALLOWED, EXTRA_CODE_METHOD_PRACK)));
    EXPECT_CALL(objUiNotifier,
            SendStartFailed(CallReasonInfo(CODE_SIP_METHOD_NOT_ALLOWED, EXTRA_CODE_METHOD_PRACK)));

    EXPECT_EQ(CallStateName::TERMINATING, pOutgoingState->SessionPrackDeliveryFailed(&objSession));
}

TEST_F(OutgoingStateTest, SessionPrackDeliveryFailedByNoResponseTerminatesCall)
{
    ON_CALL(*pConfigurationManager, IsIgnorePrackDeliveryFailure).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMessageUtils, GetResponseStatusCode(&objSession, IMessage::SESSION_PRACK, -1))
            .WillByDefault(Return(SipStatusCode::SC_INVALID));

    EXPECT_CALL(objMtcSession,
            Terminate(_, CallReasonInfo(CODE_NETWORK_RESP_TIMEOUT, EXTRA_CODE_METHOD_PRACK)));
    EXPECT_CALL(objUiNotifier,
            SendStartFailed(CallReasonInfo(CODE_NETWORK_RESP_TIMEOUT, EXTRA_CODE_METHOD_PRACK)));

    EXPECT_EQ(CallStateName::TERMINATING, pOutgoingState->SessionPrackDeliveryFailed(&objSession));
}

TEST_F(OutgoingStateTest, SessionProvisionalResponseReceivedStopsTimersIfNot100)
{
    const AString strSupportedOptionTag("supportedExtension");
    ON_CALL(objMtcSession, GetExtensionSet)
            .WillByDefault(ReturnRef(*GetTestExtensionSet(strSupportedOptionTag)));

    ON_CALL(objMessageUtils, GetResponseStatusCode(&objSession, IMessage::SESSION_START, 0))
            .WillByDefault(Return(SipStatusCode::SC_183));
    ON_CALL(objTimer, IsActive(MtcCallState::TimerType::TIMER_MO_100_WAIT))
            .WillByDefault(Return(IMS_TRUE));
    EXPECT_CALL(objTimer, Stop(MtcCallState::TimerType::TIMER_MO_100_WAIT));

    ON_CALL(objTimer, IsActive(MtcCallState::TimerType::TIMER_MO_18X_WAIT))
            .WillByDefault(Return(IMS_TRUE));
    EXPECT_CALL(objTimer, Stop(MtcCallState::TimerType::TIMER_MO_18X_WAIT));

    pOutgoingState->SessionProvisionalResponseReceived(&objSession, 0);
}

TEST_F(OutgoingStateTest, SessionProvisionalResponseReceivedStops100WaitTimerIf100)
{
    const AString strSupportedOptionTag("supportedExtension");
    ON_CALL(objMtcSession, GetExtensionSet)
            .WillByDefault(ReturnRef(*GetTestExtensionSet(strSupportedOptionTag)));

    ON_CALL(objMessageUtils, GetResponseStatusCode(&objSession, IMessage::SESSION_START, 0))
            .WillByDefault(Return(SipStatusCode::SC_100));
    ON_CALL(objTimer, IsActive(MtcCallState::TimerType::TIMER_MO_100_WAIT))
            .WillByDefault(Return(IMS_TRUE));
    EXPECT_CALL(objTimer, Stop(MtcCallState::TimerType::TIMER_MO_100_WAIT));

    ON_CALL(objTimer, IsActive(MtcCallState::TimerType::TIMER_MO_18X_WAIT))
            .WillByDefault(Return(IMS_TRUE));
    EXPECT_CALL(objTimer, Stop(MtcCallState::TimerType::TIMER_MO_18X_WAIT)).Times(0);

    pOutgoingState->SessionProvisionalResponseReceived(&objSession, 0);
}

TEST_F(OutgoingStateTest, SessionProvisionalResponseReceivedStartsUdpKeepAliveSenderIfSupported)
{
    const AString strSupportedOptionTag("supportedExtension");
    ON_CALL(objMtcSession, GetExtensionSet)
            .WillByDefault(ReturnRef(*GetTestExtensionSet(strSupportedOptionTag)));

    EXPECT_CALL(objTimer, Start(MtcCallState::TimerType::TIMER_MO_NOANSWER, _)).Times(1);

    MockUdpKeepAliveSender objKeepAliveSender(objCallContext);
    ON_CALL(objCallContext, GetUdpKeepAliveSender).WillByDefault(ReturnRef(objKeepAliveSender));
    ON_CALL(*pConfigurationManager, GetSendUdpKeepAliveIntervalTime).WillByDefault(Return(2000));
    EXPECT_CALL(objKeepAliveSender, Start);

    pOutgoingState->SessionProvisionalResponseReceived(&objSession, 0);
}

TEST_F(OutgoingStateTest,
        SessionProvisionalResponseReceivedTerminatesCallIfRequiredExtensionIsNotSupported)
{
    const AString strSupportedOptionTag("supportedExtension");
    ON_CALL(objMtcSession, GetExtensionSet)
            .WillByDefault(ReturnRef(*GetTestExtensionSet(strSupportedOptionTag)));

    MockIMessage objMessage;
    ON_CALL(objMessageUtils, GetPreviousResponse(&objSession, IMessage::SESSION_START, 0))
            .WillByDefault(Return(&objMessage));

    const AString strUnsupportedExtension("unsupportedExtension");
    ImsList<AString> lstRequiredExtensions;
    lstRequiredExtensions.Append(strUnsupportedExtension);
    ON_CALL(objMessageUtils, GetHeaders(&objMessage, ISipHeader::REQUIRE, _))
            .WillByDefault(Return(lstRequiredExtensions));

    EXPECT_CALL(objMtcSession, Terminate(_, CallReasonInfo(CODE_LOCAL_SERVICE_UNAVAILABLE)));
    EXPECT_CALL(objUiNotifier, SendStartFailed(CallReasonInfo(CODE_LOCAL_SERVICE_UNAVAILABLE)));

    EXPECT_EQ(CallStateName::TERMINATING,
            pOutgoingState->SessionProvisionalResponseReceived(&objSession, 0));
}

TEST_F(OutgoingStateTest, SessionProvisionalResponseReceivedReturnsOutgoingStateIf199)
{
    const AString strSupportedOptionTag("supportedExtension");
    ON_CALL(objMtcSession, GetExtensionSet)
            .WillByDefault(ReturnRef(*GetTestExtensionSet(strSupportedOptionTag)));
    ON_CALL(objMessageUtils, GetResponseStatusCode(&objSession, IMessage::SESSION_START, 0))
            .WillByDefault(Return(SipStatusCode::SC_199));
    MockIMessage objMessage;
    ON_CALL(objMessageUtils, GetPreviousResponse(&objSession, IMessage::SESSION_START, 0))
            .WillByDefault(Return(&objMessage));

    EXPECT_EQ(CallStateName::OUTGOING,
            pOutgoingState->SessionProvisionalResponseReceived(&objSession, 0));
}

TEST_F(OutgoingStateTest, SessionProvisionalResponseReceivedTerminatesCallIfSdpOaFails)
{
    const AString strSupportedOptionTag("supportedExtension");
    ON_CALL(objMtcSession, GetExtensionSet)
            .WillByDefault(ReturnRef(*GetTestExtensionSet(strSupportedOptionTag)));
    ON_CALL(objMessageUtils, GetResponseStatusCode(&objSession, IMessage::SESSION_START, 0))
            .WillByDefault(Return(SipStatusCode::SC_180));
    MockIMessage objMessage;
    ON_CALL(objMessageUtils, GetPreviousResponse(&objSession, IMessage::SESSION_START, 0))
            .WillByDefault(Return(&objMessage));
    EXPECT_CALL(objMediaManager, UpdatePemType(&objSession, &objMessage));

    SetSdpOaFailure(objMessage);

    EXPECT_CALL(objMtcSession, Terminate(_, CallReasonInfo(CODE_MEDIA_NOT_ACCEPTABLE)));
    EXPECT_CALL(objUiNotifier, SendStartFailed(CallReasonInfo(CODE_MEDIA_NOT_ACCEPTABLE)));
    EXPECT_EQ(CallStateName::TERMINATING,
            pOutgoingState->SessionProvisionalResponseReceived(&objSession, 0));
}

TEST_F(OutgoingStateTest, SessionProvisionalResponseReceivedInvokesSendProgressing)
{
    const AString strSupportedOptionTag("supportedExtension");
    ON_CALL(objMtcSession, GetExtensionSet)
            .WillByDefault(ReturnRef(*GetTestExtensionSet(strSupportedOptionTag)));
    ON_CALL(objMessageUtils, GetResponseStatusCode(&objSession, IMessage::SESSION_START, 0))
            .WillByDefault(Return(SipStatusCode::SC_180));
    MockIMessage objMessage;
    ON_CALL(objMessageUtils, GetPreviousResponse(&objSession, IMessage::SESSION_START, 0))
            .WillByDefault(Return(&objMessage));
    ON_CALL(objMessageUtils, GetHeaderValue(&objMessage, ISipHeader::SESSION_ID, _))
            .WillByDefault(Return(SESSION_ID));
    EXPECT_CALL(objMediaManager, UpdatePemType(&objSession, &objMessage));

    EXPECT_CALL(objPreconditionManager, OnMessageReceived(&objSession, &objMessage));
    EXPECT_CALL(objMediaManager, Run(&objSession, &objMessage, IMS_TRUE));
    EXPECT_CALL(objUiNotifier, SendProgressing());

    EXPECT_EQ(CallStateName::OUTGOING,
            pOutgoingState->SessionProvisionalResponseReceived(&objSession, 0));

    EXPECT_TRUE(pSupplementaryService->Get(SuppType::SESSION_ID));
}

TEST_F(OutgoingStateTest, SessionRprReceivedStopsTimers)
{
    const AString strSupportedOptionTag("supportedExtension");
    ON_CALL(objMtcSession, GetExtensionSet)
            .WillByDefault(ReturnRef(*GetTestExtensionSet(strSupportedOptionTag)));

    MockIMessage objMessage;
    ON_CALL(objMessageUtils, GetPreviousResponse(&objSession, IMessage::SESSION_START, 0))
            .WillByDefault(Return(&objMessage));

    ON_CALL(objTimer, IsActive(MtcCallState::TimerType::TIMER_MO_100_WAIT))
            .WillByDefault(Return(IMS_TRUE));
    EXPECT_CALL(objTimer, Stop(MtcCallState::TimerType::TIMER_MO_100_WAIT));

    ON_CALL(objTimer, IsActive(MtcCallState::TimerType::TIMER_MO_18X_WAIT))
            .WillByDefault(Return(IMS_TRUE));
    EXPECT_CALL(objTimer, Stop(MtcCallState::TimerType::TIMER_MO_18X_WAIT));

    pOutgoingState->SessionRprReceived(&objSession, 0);
}

TEST_F(OutgoingStateTest, SessionRprReceivedStopOrStartMoNoanswerTimerByContext)
{
    const AString strSupportedOptionTag("supportedExtension");
    ON_CALL(objMtcSession, GetExtensionSet)
            .WillByDefault(ReturnRef(*GetTestExtensionSet(strSupportedOptionTag)));

    MockIMessage objMessage;
    ON_CALL(objMessageUtils, GetPreviousResponse(&objSession, IMessage::SESSION_START, 0))
            .WillByDefault(Return(&objMessage));

    // config is true
    ON_CALL(*pConfigurationManager, IsStopRingbackTimerBy183WithSdpBody)
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objMessage, GetStatusCode).WillByDefault(Return(SipStatusCode::SC_183));
    MockISipMessage objSipMessage;
    ON_CALL(objMessage, GetMessage).WillByDefault(Return(&objSipMessage));
    SetSdpOaSuccessWithSdp(objMessage, objSipMessage);  // to make IMessage have SDP

    ON_CALL(objTimer, IsActive(MtcCallState::TimerType::TIMER_MO_NOANSWER))
            .WillByDefault(Return(IMS_TRUE));
    EXPECT_CALL(objTimer, Stop(MtcCallState::TimerType::TIMER_MO_NOANSWER));

    pOutgoingState->SessionRprReceived(&objSession, 0);

    // config is false
    ON_CALL(*pConfigurationManager, IsStopRingbackTimerBy183WithSdpBody)
            .WillByDefault(Return(IMS_FALSE));

    EXPECT_CALL(objTimer, Start(MtcCallState::TimerType::TIMER_MO_NOANSWER, _));

    pOutgoingState->SessionRprReceived(&objSession, 0);
}

TEST_F(OutgoingStateTest, SessionRprReceivedTerminatesCallIfRequiredExtensionIsNotSupported)
{
    const AString strSupportedOptionTag("supportedExtension");
    ON_CALL(objMtcSession, GetExtensionSet)
            .WillByDefault(ReturnRef(*GetTestExtensionSet(strSupportedOptionTag)));

    MockIMessage objMessage;
    ON_CALL(objMessageUtils, GetPreviousResponse(&objSession, IMessage::SESSION_START, 0))
            .WillByDefault(Return(&objMessage));

    const AString strUnsupportedExtension("unsupportedExtension");
    ImsList<AString> lstRequiredExtensions;
    lstRequiredExtensions.Append(strUnsupportedExtension);
    ON_CALL(objMessageUtils, GetHeaders(&objMessage, ISipHeader::REQUIRE, _))
            .WillByDefault(Return(lstRequiredExtensions));

    EXPECT_CALL(objMtcSession, Terminate(_, CallReasonInfo(CODE_LOCAL_SERVICE_UNAVAILABLE)));
    EXPECT_CALL(objUiNotifier, SendStartFailed(CallReasonInfo(CODE_LOCAL_SERVICE_UNAVAILABLE)));

    EXPECT_EQ(CallStateName::TERMINATING, pOutgoingState->SessionRprReceived(&objSession, 0));
}

TEST_F(OutgoingStateTest, SessionRprReceivedReturnsOutgoingStateIf199)
{
    const AString strSupportedOptionTag("supportedExtension");
    ON_CALL(objMtcSession, GetExtensionSet)
            .WillByDefault(ReturnRef(*GetTestExtensionSet(strSupportedOptionTag)));
    ON_CALL(objMessageUtils, GetResponseStatusCode(&objSession, IMessage::SESSION_START, 0))
            .WillByDefault(Return(SipStatusCode::SC_199));
    MockIMessage objMessage;
    ON_CALL(objMessageUtils, GetPreviousResponse(&objSession, IMessage::SESSION_START, 0))
            .WillByDefault(Return(&objMessage));

    EXPECT_EQ(CallStateName::OUTGOING, pOutgoingState->SessionRprReceived(&objSession, 0));
}

TEST_F(OutgoingStateTest, SessionRprReceivedTerminatesCallIfSdpOaFails)
{
    const AString strSupportedOptionTag("supportedExtension");
    ON_CALL(objMtcSession, GetExtensionSet)
            .WillByDefault(ReturnRef(*GetTestExtensionSet(strSupportedOptionTag)));
    ON_CALL(objMessageUtils, GetResponseStatusCode(&objSession, IMessage::SESSION_START, 0))
            .WillByDefault(Return(SipStatusCode::SC_180));
    MockIMessage objMessage;
    ON_CALL(objMessageUtils, GetPreviousResponse(&objSession, IMessage::SESSION_START, 0))
            .WillByDefault(Return(&objMessage));
    EXPECT_CALL(objMediaManager, UpdatePemType(&objSession, &objMessage));

    SetSdpOaFailure(objMessage);

    EXPECT_CALL(objMtcSession, Terminate(_, CallReasonInfo(CODE_MEDIA_NOT_ACCEPTABLE)));
    EXPECT_CALL(objUiNotifier, SendStartFailed(CallReasonInfo(CODE_MEDIA_NOT_ACCEPTABLE)));
    EXPECT_EQ(CallStateName::TERMINATING, pOutgoingState->SessionRprReceived(&objSession, 0));
}

TEST_F(OutgoingStateTest, SessionRprReceivedUpdatesQosPreconditionInfo)
{
    const AString strSupportedOptionTag("supportedExtension");
    ON_CALL(objMtcSession, GetExtensionSet)
            .WillByDefault(ReturnRef(*GetTestExtensionSet(strSupportedOptionTag)));
    ON_CALL(objMessageUtils, GetResponseStatusCode(&objSession, IMessage::SESSION_START, 0))
            .WillByDefault(Return(SipStatusCode::SC_180));
    MockIMessage objMessage;
    ON_CALL(objMessageUtils, GetPreviousResponse(&objSession, IMessage::SESSION_START, 0))
            .WillByDefault(Return(&objMessage));

    MockISipMessage objSipMessage;
    ON_CALL(objMessage, GetMessage).WillByDefault(Return(&objSipMessage));
    SetSdpOaSuccessWithSdp(objMessage, objSipMessage);  // to cover OnMessageReceived()
    EXPECT_CALL(objPreconditionManager, OnMessageReceived(&objSession, &objMessage));

    EXPECT_EQ(CallStateName::OUTGOING, pOutgoingState->SessionRprReceived(&objSession, 0));
}

TEST_F(OutgoingStateTest, SessionRprReceivedTerminatesCallIfRemoteAudioPortIsZero)
{
    const AString strSupportedOptionTag("supportedExtension");
    ON_CALL(objMtcSession, GetExtensionSet)
            .WillByDefault(ReturnRef(*GetTestExtensionSet(strSupportedOptionTag)));
    ON_CALL(objMessageUtils, GetResponseStatusCode(&objSession, IMessage::SESSION_START, 0))
            .WillByDefault(Return(SipStatusCode::SC_180));
    MockIMessage objMessage;
    ON_CALL(objMessageUtils, GetPreviousResponse(&objSession, IMessage::SESSION_START, 0))
            .WillByDefault(Return(&objMessage));

    ON_CALL(objMediaManager, GetRemoteRtpPort(_, MEDIATYPE_AUDIO)).WillByDefault(Return(0));

    EXPECT_CALL(objMtcSession,
            Terminate(_,
                    CallReasonInfo(CODE_LOCAL_CALL_CS_RETRY_REQUIRED,
                            EXTRA_CODE_CALL_RETRY_SILENT_REDIAL)));
    EXPECT_CALL(objUiNotifier,
            SendStartFailed(CallReasonInfo(
                    CODE_LOCAL_CALL_CS_RETRY_REQUIRED, EXTRA_CODE_CALL_RETRY_SILENT_REDIAL)));
    EXPECT_EQ(CallStateName::TERMINATING, pOutgoingState->SessionRprReceived(&objSession, 0));
}

TEST_F(OutgoingStateTest, SessionRprReceivedTerminatesCallIfSendingPrackFails)
{
    const AString strSupportedOptionTag("supportedExtension");
    ON_CALL(objMtcSession, GetExtensionSet)
            .WillByDefault(ReturnRef(*GetTestExtensionSet(strSupportedOptionTag)));
    ON_CALL(objMessageUtils, GetResponseStatusCode(&objSession, IMessage::SESSION_START, 0))
            .WillByDefault(Return(SipStatusCode::SC_180));
    MockIMessage objMessage;
    ON_CALL(objMessageUtils, GetPreviousResponse(&objSession, IMessage::SESSION_START, 0))
            .WillByDefault(Return(&objMessage));

    ON_CALL(objMtcSession, SendPrack(IMS_FALSE)).WillByDefault(Return(IMS_FAILURE));

    EXPECT_CALL(objMtcSession, Terminate(_, CallReasonInfo(CODE_LOCAL_INTERNAL_ERROR)));
    EXPECT_CALL(objUiNotifier, SendStartFailed(CallReasonInfo(CODE_LOCAL_INTERNAL_ERROR)));
    EXPECT_EQ(CallStateName::TERMINATING, pOutgoingState->SessionRprReceived(&objSession, 0));
}

TEST_F(OutgoingStateTest, SessionRprReceivedInvokesSendProgressing)
{
    const AString strSupportedOptionTag("supportedExtension");
    ON_CALL(objMtcSession, GetExtensionSet)
            .WillByDefault(ReturnRef(*GetTestExtensionSet(strSupportedOptionTag)));
    ON_CALL(objMessageUtils, GetResponseStatusCode(&objSession, IMessage::SESSION_START, 0))
            .WillByDefault(Return(SipStatusCode::SC_183));
    MockIMessage objMessage;
    ON_CALL(objMessageUtils, GetPreviousResponse(&objSession, IMessage::SESSION_START, 0))
            .WillByDefault(Return(&objMessage));
    ON_CALL(objMessageUtils, GetHeaderValue(&objMessage, ISipHeader::SESSION_ID, _))
            .WillByDefault(Return(SESSION_ID));

    ON_CALL(objMtcSession, SendPrack(IMS_FALSE)).WillByDefault(Return(IMS_SUCCESS));

    EXPECT_CALL(objPreconditionManager, OnMessageReceived(&objSession, &objMessage));
    EXPECT_CALL(objMediaManager, Run(&objSession, &objMessage, IMS_TRUE));
    EXPECT_CALL(objUiNotifier, SendProgressing());

    EXPECT_EQ(CallStateName::OUTGOING, pOutgoingState->SessionRprReceived(&objSession, 0));

    EXPECT_TRUE(pSupplementaryService->Get(SuppType::SESSION_ID));
}

TEST_F(OutgoingStateTest, SessionRprReceivedInvokesStartWatchdogIfSupportedAndSdpAnswerIncluded)
{
    const AString strSupportedOptionTag("supportedExtension");
    ON_CALL(objMtcSession, GetExtensionSet)
            .WillByDefault(ReturnRef(*GetTestExtensionSet(strSupportedOptionTag)));
    ON_CALL(objMessageUtils, GetResponseStatusCode(&objSession, IMessage::SESSION_START, 0))
            .WillByDefault(Return(SipStatusCode::SC_183));
    MockIMessage objMessage;
    ON_CALL(objMessageUtils, GetPreviousResponse(&objSession, IMessage::SESSION_START, 0))
            .WillByDefault(Return(&objMessage));

    ON_CALL(objMtcSession, SendPrack(IMS_FALSE)).WillByDefault(Return(IMS_SUCCESS));

    MockISipMessage objSipMessage;
    ON_CALL(objMessage, GetMessage).WillByDefault(Return(&objSipMessage));
    SetSdpOaSuccessWithSdp(objMessage, objSipMessage);  // to cover StartEpsFallbackWatchdogIfNeeded

    ON_CALL(*pConfigurationManager, GetEpsFallbackWatchdogTime).WillByDefault(Return(2000));
    ON_CALL(objService, IsNr).WillByDefault(Return(IMS_TRUE));

    EXPECT_CALL(*pEpsFbTrigger, StartWatchdog);

    EXPECT_EQ(CallStateName::OUTGOING, pOutgoingState->SessionRprReceived(&objSession, 0));
}

TEST_F(OutgoingStateTest,
        SessionRprReceivedDoesNotSendLocalResourceConfirmationWithPrackIfNotRequired)
{
    const AString strSupportedOptionTag("supportedExtension");
    ON_CALL(objMtcSession, GetExtensionSet)
            .WillByDefault(ReturnRef(*GetTestExtensionSet(strSupportedOptionTag)));
    ON_CALL(objMessageUtils, GetResponseStatusCode(&objSession, IMessage::SESSION_START, 0))
            .WillByDefault(Return(SipStatusCode::SC_183));
    MockIMessage objMessage;
    ON_CALL(objMessageUtils, GetPreviousResponse(&objSession, IMessage::SESSION_START, 0))
            .WillByDefault(Return(&objMessage));

    ON_CALL(objPreconditionManager, IsLocalResourceConfirmationRequired(&objSession))
            .WillByDefault(Return(IMS_FALSE));

    EXPECT_CALL(objMtcSession, SendPrack(IMS_FALSE)).WillOnce(Return(IMS_SUCCESS));
    EXPECT_CALL(objPreconditionManager, OnMessageReceived(&objSession, &objMessage));
    EXPECT_CALL(objMediaManager, Run(&objSession, &objMessage, IMS_TRUE));
    EXPECT_CALL(objUiNotifier, SendProgressing());

    EXPECT_EQ(CallStateName::OUTGOING, pOutgoingState->SessionRprReceived(&objSession, 0));
}

TEST_F(OutgoingStateTest,
        SessionRprReceivedDoesNotSendLocalResourceConfirmationWithPrackIfResourceNotReserved)
{
    const AString strSupportedOptionTag("supportedExtension");
    ON_CALL(objMtcSession, GetExtensionSet)
            .WillByDefault(ReturnRef(*GetTestExtensionSet(strSupportedOptionTag)));
    ON_CALL(objMessageUtils, GetResponseStatusCode(&objSession, IMessage::SESSION_START, 0))
            .WillByDefault(Return(SipStatusCode::SC_183));
    MockIMessage objMessage;
    ON_CALL(objMessageUtils, GetPreviousResponse(&objSession, IMessage::SESSION_START, 0))
            .WillByDefault(Return(&objMessage));

    ON_CALL(objPreconditionManager, IsLocalResourceConfirmationRequired(&objSession))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objPreconditionManager, IsAvailableToSendLocalResourceConfirmation(&objSession))
            .WillByDefault(Return(IMS_FALSE));

    EXPECT_CALL(objMtcSession, SendPrack(IMS_FALSE)).WillOnce(Return(IMS_SUCCESS));
    EXPECT_CALL(objPreconditionManager, OnMessageReceived(&objSession, &objMessage));
    EXPECT_CALL(objMediaManager, Run(&objSession, &objMessage, IMS_TRUE));
    EXPECT_CALL(objUiNotifier, SendProgressing());

    EXPECT_EQ(CallStateName::OUTGOING, pOutgoingState->SessionRprReceived(&objSession, 0));
}

TEST_F(OutgoingStateTest,
        SessionRprReceivedInvokesLocalResourceConfirmationWithPrackIfResourceReserved)
{
    const AString strSupportedOptionTag("supportedExtension");
    ON_CALL(objMtcSession, GetExtensionSet)
            .WillByDefault(ReturnRef(*GetTestExtensionSet(strSupportedOptionTag)));
    ON_CALL(objMessageUtils, GetResponseStatusCode(&objSession, IMessage::SESSION_START, 0))
            .WillByDefault(Return(SipStatusCode::SC_183));
    MockIMessage objMessage;
    ON_CALL(objMessageUtils, GetPreviousResponse(&objSession, IMessage::SESSION_START, 0))
            .WillByDefault(Return(&objMessage));

    ON_CALL(objPreconditionManager, IsLocalResourceConfirmationRequired(&objSession))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objPreconditionManager, IsAvailableToSendLocalResourceConfirmation(&objSession))
            .WillByDefault(Return(IMS_TRUE));

    EXPECT_CALL(objMtcSession, SendPrack(IMS_TRUE)).WillOnce(Return(IMS_SUCCESS));
    EXPECT_CALL(objPreconditionManager, OnMessageReceived(&objSession, &objMessage));
    EXPECT_CALL(objMediaManager, Run(&objSession, &objMessage, IMS_TRUE));
    EXPECT_CALL(objUiNotifier, SendProgressing());

    EXPECT_EQ(CallStateName::OUTGOING, pOutgoingState->SessionRprReceived(&objSession, 0));
}

TEST_F(OutgoingStateTest, UssiStartedInvokesSendStartedToUi)
{
    MockIMessage objMessage;
    ON_CALL(objMessageUtils, GetPreviousResponse(&objSession, IMessage::SESSION_START, -1))
            .WillByDefault(Return(&objMessage));
    ON_CALL(objMtcSession, SendAck).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(objMessage, GetStatusCode).WillByDefault(Return(SipStatusCode::SC_200));

    EXPECT_CALL(objUiNotifier, SendStarted);

    EXPECT_EQ(CallStateName::ESTABLISHED, pOutgoingState->UssiStarted(&objSession));
}
