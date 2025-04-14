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

#include "AString.h"
#include "CallReasonInfo.h"
#include "CarrierConfig.h"
#include "Engine.h"
#include "IConfiguration.h"
#include "INetworkWatcher.h"
#include "IImsRadio.h"
#include "ISession.h"
#include "ISipHeader.h"
#include "Ims3gpp.h"
#include "ImsAosReason.h"
#include "ImsVector.h"
#include "MockIMessage.h"
#include "MockIMtcCallController.h"
#include "MockIMtcService.h"
#include "MockISession.h"
#include "MockISipKeepAliveHelper.h"
#include "MockISipMessage.h"
#include "MtcDef.h"
#include "PlatformContext.h"
#include "SipStatusCode.h"
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
#include "call/radio/MockIMtcRadioChecker.h"
#include "call/state/MtcCallState.h"
#include "call/state/OutgoingState.h"
#include "configuration/MockMtcConfigurationProxy.h"
#include "configuration/MtcConfigurationProxy.h"
#include "dialingplan/MockIMtcDialingPlan.h"
#include "helper/ISrvccStateListener.h"
#include "helper/MockIMtcAosConnector.h"
#include "helper/MockIPassiveTimerHolder.h"
#include "helper/MockMtcTimerWrapper.h"
#include "helper/MockUdpKeepAliveSender.h"
#include "helper/MtcSupplementaryService.h"
#include "helper/sipinterfaceholder/MockIMtcSipInterfaceFactory.h"
#include "helper/sipinterfaceholder/MockSessionInterfaceHolder.h"
#include "media/MockIMtcMediaManager.h"
#include "precondition/MockIMtcPreconditionManager.h"
#include "precondition/QosDef.h"
#include "utility/MockIMessageUtils.h"
#include <gtest/gtest.h>
#include <initializer_list>

using ::testing::_;
using ::testing::AnyNumber;
using ::testing::Ref;
using ::testing::Return;
using ::testing::ReturnRef;

static const IMS_CHAR SESSION_ID[] = "f81d4fae7dec11d0a76500a0c91e6bf6";

class OutgoingStateTest : public ::testing::Test
{
public:
    OutgoingState* pOutgoingState;
    MockIMtcSession objMtcSession;
    MockMtcConfigurationProxy* pConfigurationProxy;
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
    MockIPassiveTimerHolder objPassiveTimer;
    MockIMtcUiNotifier objUiNotifier;
    MockIMessageUtils objMessageUtils;
    MockEpsFallbackTrigger* pEpsFbTrigger;
    MockIMtcSipInterfaceFactory objSipInterfaceFactory;
    MockSessionInterfaceHolder* pSessionInterfaceHolder;
    MockISipKeepAliveHelper objKeepAliveHelper;
    MockIMtcRadioChecker objRadioChecker;
    SipMethod objAckMethod;
    SipMethod objInviteMethod;
    MtcSupplementaryService* pSupplementaryService;
    MediaInfo objMediaInfo;
    ImsList<IMtcSession*> objSessions;
    MockIMtcCallManager objCallManager;
    TestConfigService* pConfigService;
    ImsVector<AString> objActionSets;

protected:
    virtual void SetUp() override
    {
        pConfigService = new TestConfigService();
        pConfigService->SetCarrierConfig(&(pConfigService->GetMockCarrierConfig()));
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_CONFIG, pConfigService);

        objAckMethod = SipMethod::ACK;
        objInviteMethod = SipMethod::INVITE;

        pConfigurationProxy = new MockMtcConfigurationProxy();
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
        ON_CALL(objCallContext, GetPassiveTimerHolder).WillByDefault(ReturnRef(objPassiveTimer));
        ON_CALL(objCallContext, GetMediaManager).WillByDefault(ReturnRef(objMediaManager));
        ON_CALL(objCallContext, GetMessageUtils).WillByDefault(ReturnRef(objMessageUtils));
        ON_CALL(objCallContext, GetCallManager).WillByDefault(ReturnRef(objCallManager));
        ON_CALL(objCallContext, GetRadioChecker).WillByDefault(ReturnRef(objRadioChecker));

        pEpsFbTrigger = new MockEpsFallbackTrigger(objCallContext);
        ON_CALL(objCallContext, GetEpsFallbackTrigger).WillByDefault(ReturnRef(*pEpsFbTrigger));

        ON_CALL(objService, GetAosConnector).WillByDefault(Return(&objAosConnector));

        ON_CALL(objMtcSession, GetISession).WillByDefault(ReturnRef(objSession));
        ON_CALL(objSession, IsFinalResponseReceivedForInitialInviteRequest)
                .WillByDefault(Return(IMS_FALSE));

        ON_CALL(objCallContext, GetPreconditionManager)
                .WillByDefault(ReturnRef(objPreconditionManager));

        pSessionInterfaceHolder = new MockSessionInterfaceHolder();
        ON_CALL(objSipInterfaceFactory, GetISessionHolder)
                .WillByDefault(ReturnRef(*pSessionInterfaceHolder));
        ON_CALL(objCallContext, GetSipInterfaceFactory)
                .WillByDefault(ReturnRef(objSipInterfaceFactory));

        ON_CALL(objMediaManager, GetMediaInfo).WillByDefault(ReturnRef(objMediaInfo));
        ON_CALL(objMediaManager, GetRemoteRtpPort(_, MEDIATYPE_AUDIO)).WillByDefault(Return(12345));

        objCallInfo.ePeerType = PeerType::MO;

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

    void SetNegotiateSdpFailure(
            IN MockIMessage& objMessage, IN const NegotiationResult& eNegoResult)
    {
        ON_CALL(objMessageUtils, HasSdp(&objMessage)).WillByDefault(Return(IMS_TRUE));
        ON_CALL(objMediaManager, GetNegotiationState(&objSession))
                .WillByDefault(Return(NegotiationState::STATE_OFFER_SENT));
        ON_CALL(objMessage, GetMethod).WillByDefault(ReturnRef(objAckMethod));
        ON_CALL(objMediaManager, NegotiateSdp(&objSession)).WillByDefault(Return(eNegoResult));
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

    void SetUpStartErrorHandler(IN MockIMessage* pMessage, IN IMS_SINT32 eStatusCode,
            IN IMS_BOOL bCsfb, IN IMS_SINT32 nPolicyOfTimerB, IN IMS_BOOL bWiFi)
    {
        if (pMessage)
        {
            ON_CALL(*pMessage, GetStatusCode).WillByDefault(Return(eStatusCode));
        }

        ON_CALL(objService, IsWlanIpCanType).WillByDefault(Return(bWiFi));
        ON_CALL(objCallContext, IsCsfbAvailable).WillByDefault(Return(bCsfb));
        if (bWiFi)
        {
            ON_CALL(*pConfigurationProxy,
                    GetInt(ConfigWfc::KEY_POLICY_FOR_TCALL_TIMER_EXPIRY_OF_VOWIFI_CALL_INT))
                    .WillByDefault(Return(nPolicyOfTimerB));
        }
        else
        {
            ON_CALL(*pConfigurationProxy,
                    GetInt(ConfigVoice::KEY_POLICY_FOR_TCALL_TIMER_EXPIRY_OF_VOLTE_CALL_INT))
                    .WillByDefault(Return(nPolicyOfTimerB));
        }
    }

    void SetActionConfigs(IN const IMS_CHAR* pszKey, IN IMS_SINT32 nStatusCode,
            IN std::initializer_list<IMS_SINT32> objActions)
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
        ON_CALL(*pConfigurationProxy, GetStringArray(pszKey)).WillByDefault(Return(objActionSets));
    }

    MtcExtensionSet GetTestExtensionSet(IN const AString& strOptionTag)
    {
        ImsList<IMtcExtension*> objExtensions;
        MockIMtcExtension* pExtension = new MockIMtcExtension();
        ON_CALL(*pExtension, GetOptionTag).WillByDefault(ReturnRef(strOptionTag));
        ON_CALL(*pExtension, IsAvailableOnRemote).WillByDefault(Return(IMS_TRUE));
        objExtensions.Append(pExtension);
        MtcExtensionSet objMtcExtensionSet(objCallContext, objExtensions);
        return objMtcExtensionSet;
    }
};

TEST_F(OutgoingStateTest, OnExitStopsUdpKeepAliveSenderIfSupported)
{
    MtcExtensionSet objMtcExtensionSet(GetTestExtensionSet(AString("supportedExtension")));
    ON_CALL(objMtcSession, GetExtensionSet).WillByDefault(ReturnRef(objMtcExtensionSet));

    MockUdpKeepAliveSender* pKeepAliveSender = new MockUdpKeepAliveSender(
            &objKeepAliveHelper, objCallContext);  // OutgoingState deletes it
    ON_CALL(objCallContext, CreateUdpKeepAliveSender).WillByDefault(Return(pKeepAliveSender));
    ON_CALL(*pConfigurationProxy,
            GetInt(ConfigVoice::KEY_SEND_UDP_KEEP_ALIVE_INTERVAL_TIME_MILLIS_INT))
            .WillByDefault(Return(2000));
    pOutgoingState->SessionProvisionalResponseReceived(&objSession, 0);

    EXPECT_CALL(*pKeepAliveSender, Stop);
    pOutgoingState->OnExit();
}

TEST_F(OutgoingStateTest, ResponseWaitTimeoutForReasonExpirationUpdatesReason)
{
    const CallReasonInfo objReasonNone(CODE_NONE);
    const CallReasonInfo objReasonUserTerminated(CODE_USER_TERMINATED);
    const CallReasonInfo objReasonSipTimeout(
            CODE_USER_TERMINATED, EXTRA_USER_TERMINATED_AND_SIP_TIMEOUT);

    EXPECT_CALL(objMtcSession, Terminate(_, objReasonNone));
    EXPECT_CALL(objUiNotifier, SendStartFailed(objReasonNone));
    pOutgoingState->Terminate(objReasonNone);

    ON_CALL(*pConfigurationProxy,
            GetInt(ConfigVoice::KEY_USER_CANCEL_REASON_AFTER_RESPONSE_TIMEOUT_TIMER_MILLIS_INT))
            .WillByDefault(Return(0));
    EXPECT_CALL(objMtcSession, Terminate(_, objReasonUserTerminated));
    EXPECT_CALL(objUiNotifier, SendStartFailed(objReasonUserTerminated));
    pOutgoingState->Terminate(objReasonUserTerminated);

    ON_CALL(*pConfigurationProxy,
            GetInt(ConfigVoice::KEY_USER_CANCEL_REASON_AFTER_RESPONSE_TIMEOUT_TIMER_MILLIS_INT))
            .WillByDefault(Return(8000));
    EXPECT_CALL(objMtcSession, Terminate(_, objReasonUserTerminated));
    EXPECT_CALL(objUiNotifier, SendStartFailed(objReasonUserTerminated));
    pOutgoingState->Terminate(objReasonUserTerminated);

    pOutgoingState->OnTimerExpired(MtcCallState::TIMER_MO_RESPONSE_TIMEOUT_FOR_REASON);
    EXPECT_CALL(objMtcSession, Terminate(_, objReasonSipTimeout));
    EXPECT_CALL(objUiNotifier, SendStartFailed(objReasonSipTimeout));
    pOutgoingState->Terminate(objReasonUserTerminated);
}

TEST_F(OutgoingStateTest, ActAsIfTimerBExpiryWhenB1ExpiresAndCallEnds)
{
    const CallReasonInfo objReasonSipTimeout(
            CODE_USER_TERMINATED, EXTRA_USER_TERMINATED_AND_SIP_TIMEOUT);

    SetUpStartErrorHandler(IMS_NULL, SipStatusCode::SC_INVALID, IMS_FALSE,
            ConfigVoice::MO_CALL_REQUEST_TIMEOUT_POLICY_INITIAL_REGISTER_PCSCF_DISCOVERY_AFTER_CSFB,
            IMS_FALSE);
    ON_CALL(*pConfigurationProxy,
            GetInt(ConfigVoice::KEY_USER_CANCEL_REASON_AFTER_RESPONSE_TIMEOUT_TIMER_MILLIS_INT))
            .WillByDefault(Return(8000));
    pOutgoingState->OnTimerExpired(MtcCallState::TIMER_MO_RESPONSE_TIMEOUT_FOR_REASON);
    ON_CALL(objCallContext, GetSession()).WillByDefault(Return(&objMtcSession));

    EXPECT_CALL(objMtcSession, Terminate(_, objReasonSipTimeout));
    EXPECT_CALL(objUiNotifier, SendStartFailed(objReasonSipTimeout));
    pOutgoingState->Terminate(objReasonSipTimeout);
}

TEST_F(OutgoingStateTest, ActAsIfTimerBExpiryWhenB1ExpiresAndCallEndsSkipControlAos)
{
    const CallReasonInfo objReasonSipTimeout(
            CODE_USER_TERMINATED, EXTRA_USER_TERMINATED_AND_SIP_TIMEOUT);

    SetUpStartErrorHandler(IMS_NULL, SipStatusCode::SC_INVALID, IMS_FALSE,
            ConfigVoice::MO_CALL_REQUEST_TIMEOUT_POLICY_INITIAL_REGISTER_PCSCF_DISCOVERY_AFTER_CSFB,
            IMS_FALSE);
    ON_CALL(*pConfigurationProxy,
            GetInt(ConfigVoice::KEY_USER_CANCEL_REASON_AFTER_RESPONSE_TIMEOUT_TIMER_MILLIS_INT))
            .WillByDefault(Return(8000));
    pOutgoingState->OnTimerExpired(MtcCallState::TIMER_MO_RESPONSE_TIMEOUT_FOR_REASON);
    ON_CALL(objCallContext, GetSession()).WillByDefault(Return(nullptr));

    EXPECT_CALL(objUiNotifier, SendStartFailed(objReasonSipTimeout));
    pOutgoingState->Terminate(objReasonSipTimeout);
}

TEST_F(OutgoingStateTest, ActAsIfTimerBExpiryWhenB1ExpiresAndCallEndsIgnoreCsfb)
{
    const CallReasonInfo objReasonSipTimeout(
            CODE_USER_TERMINATED, EXTRA_USER_TERMINATED_AND_SIP_TIMEOUT);

    SetUpStartErrorHandler(IMS_NULL, SipStatusCode::SC_INVALID, IMS_FALSE,
            ConfigVoice::MO_CALL_REQUEST_TIMEOUT_POLICY_INITIAL_REGISTER_PCSCF_DISCOVERY_AFTER_CSFB,
            IMS_FALSE);
    ON_CALL(*pConfigurationProxy,
            GetInt(ConfigVoice::KEY_USER_CANCEL_REASON_AFTER_RESPONSE_TIMEOUT_TIMER_MILLIS_INT))
            .WillByDefault(Return(8000));
    pOutgoingState->OnTimerExpired(MtcCallState::TIMER_MO_RESPONSE_TIMEOUT_FOR_REASON);
    ON_CALL(objCallContext, GetSession()).WillByDefault(Return(&objMtcSession));
    ON_CALL(objCallContext, IsCsfbAvailable).WillByDefault(Return(IMS_TRUE));

    EXPECT_CALL(objMtcSession, Terminate(_, objReasonSipTimeout));
    EXPECT_CALL(objUiNotifier, SendStartFailed(objReasonSipTimeout));
    pOutgoingState->Terminate(objReasonSipTimeout);
}

TEST_F(OutgoingStateTest, RegistrationTimerExpiredTerminatesCall)
{
    EXPECT_CALL(objUiNotifier, SendStartFailed(CallReasonInfo(CODE_SIP_SERVER_ERROR)));

    EXPECT_EQ(CallStateName::TERMINATING,
            pOutgoingState->OnTimerExpired(MtcCallState::TIMER_MO_REGISTRATION_FOR_SILENT_REDIAL));
}

TEST_F(OutgoingStateTest, 18xTimerExpiredTerminatesCall)
{
    EXPECT_CALL(objMtcSession, Terminate(_, CallReasonInfo(CODE_TIMEOUT_1XX_WAITING)));
    EXPECT_CALL(objUiNotifier, SendStartFailed(CallReasonInfo(CODE_TIMEOUT_1XX_WAITING)));

    EXPECT_EQ(CallStateName::TERMINATING,
            pOutgoingState->OnTimerExpired(MtcCallState::TIMER_MO_18X_WAIT));
}

TEST_F(OutgoingStateTest, InviteTo18xTimerExpiredTerminatesCall)
{
    EXPECT_CALL(objMtcSession, Terminate(_, CallReasonInfo(CODE_TIMEOUT_NO_ANSWER)));
    EXPECT_CALL(objUiNotifier, SendStartFailed(CallReasonInfo(CODE_TIMEOUT_NO_ANSWER)));

    EXPECT_EQ(CallStateName::TERMINATING,
            pOutgoingState->OnTimerExpired(MtcCallState::TIMER_MO_CALL_INITIATION_TO_18X_WAIT));
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

TEST_F(OutgoingStateTest, OnConnectionFailedDoesNothingIfAlreadyReceivedResponse)
{
    MockIMessage objMessage;
    ON_CALL(objMessageUtils, GetPreviousResponse(&objSession, IMessage::SESSION_START, -1))
            .WillByDefault(Return(&objMessage));

    EXPECT_CALL(*pEpsFbTrigger, TriggerEpsFallback(EpsFallbackReason::RADIO_CHECK_BLOCK)).Times(0);
    EXPECT_CALL(objMtcSession, Terminate(_, _)).Times(0);
    EXPECT_CALL(objUiNotifier, SendStartFailed(_)).Times(0);

    EXPECT_EQ(CallStateName::OUTGOING,
            pOutgoingState->OnConnectionFailed(IImsRadio::REASON_ACCESS_DENIED, 2));
}

TEST_F(OutgoingStateTest, OnConnectionFailedTriggersEpsfbIfRequired)
{
    ON_CALL(objMessageUtils, GetPreviousResponse(&objSession, IMessage::SESSION_START, -1))
            .WillByDefault(Return(IMS_NULL));
    ON_CALL(objService, IsNr).WillByDefault(Return(IMS_TRUE));
    ON_CALL(*pConfigurationProxy,
            GetBoolean(ConfigVoice::KEY_EPS_FALLBACK_TRIGGER_BY_AC_BARRING_BOOL))
            .WillByDefault(Return(IMS_TRUE));

    EXPECT_CALL(*pEpsFbTrigger, TriggerEpsFallback(EpsFallbackReason::RADIO_CHECK_BLOCK));
    EXPECT_CALL(objMtcSession, Terminate(_, _)).Times(0);
    EXPECT_CALL(objUiNotifier, SendStartFailed(_)).Times(0);

    EXPECT_EQ(CallStateName::OUTGOING,
            pOutgoingState->OnConnectionFailed(IImsRadio::REASON_ACCESS_DENIED, 2));
}

TEST_F(OutgoingStateTest, OnConnectionFailedDoesNothingIfEpsFallbackIsNotRequiredInNr)
{
    ON_CALL(objMessageUtils, GetPreviousResponse(&objSession, IMessage::SESSION_START, -1))
            .WillByDefault(Return(IMS_NULL));
    ON_CALL(objService, IsNr).WillByDefault(Return(IMS_TRUE));

    EXPECT_CALL(*pEpsFbTrigger, TriggerEpsFallback(EpsFallbackReason::RADIO_CHECK_BLOCK)).Times(0);
    EXPECT_CALL(objMtcSession, Terminate(_, _)).Times(0);
    EXPECT_CALL(objUiNotifier, SendStartFailed(_)).Times(0);

    EXPECT_EQ(CallStateName::OUTGOING,
            pOutgoingState->OnConnectionFailed(IImsRadio::REASON_RACH_FAILURE, 2));
}

TEST_F(OutgoingStateTest, OnConnectionFailedDoesNothingIfReasonIsRrcRejectInNonNR)
{
    ON_CALL(objMessageUtils, GetPreviousResponse(&objSession, IMessage::SESSION_START, -1))
            .WillByDefault(Return(IMS_NULL));
    ON_CALL(objService, GetSrvccState).WillByDefault(Return(SrvccState::IDLE));
    ON_CALL(objService, IsNr).WillByDefault(Return(IMS_FALSE));
    SetUpStartErrorHandler(IMS_NULL, SipStatusCode::SC_INVALID, IMS_FALSE,
            ConfigVoice::MO_CALL_REQUEST_TIMEOUT_POLICY_CALL_END, IMS_FALSE);

    EXPECT_CALL(objMtcSession, Terminate(_, _)).Times(0);
    EXPECT_CALL(objUiNotifier, SendStartFailed(_)).Times(0);

    EXPECT_EQ(CallStateName::OUTGOING,
            pOutgoingState->OnConnectionFailed(IImsRadio::REASON_RRC_REJECT, 2));
}

TEST_F(OutgoingStateTest, OnConnectionInvokesCallTerminationIfReasonIsNotRrcRejectInNonNR)
{
    ON_CALL(objService, GetSrvccState).WillByDefault(Return(SrvccState::IDLE));
    ON_CALL(objService, IsNr).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMessageUtils, GetPreviousResponse(&objSession, IMessage::SESSION_START, -1))
            .WillByDefault(Return(IMS_NULL));
    SetUpStartErrorHandler(IMS_NULL, SipStatusCode::SC_INVALID, IMS_FALSE,
            ConfigVoice::MO_CALL_REQUEST_TIMEOUT_POLICY_CALL_END, IMS_FALSE);

    EXPECT_CALL(objMtcSession, Terminate(_, _)).Times(1);
    EXPECT_CALL(objUiNotifier, SendStartFailed(_)).Times(1);

    EXPECT_EQ(CallStateName::TERMINATING,
            pOutgoingState->OnConnectionFailed(IImsRadio::REASON_ACCESS_DENIED, 2));
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

TEST_F(OutgoingStateTest, HandleAosConnectedStopsRegistrationTimers)
{
    ON_CALL(objTimer, IsActive(MtcCallState::TimerType::TIMER_MO_REGISTRATION_FOR_SILENT_REDIAL))
            .WillByDefault(Return(IMS_TRUE));

    EXPECT_CALL(objTimer, Stop(MtcCallState::TimerType::TIMER_MO_REGISTRATION_FOR_SILENT_REDIAL));
    pOutgoingState->OnAosStateChanged(MtcAosState::CONNECTED, 0);
}

TEST_F(OutgoingStateTest, HandleAosConnectedDoesNotRedialIfNotWaitingEpsFallback)
{
    ON_CALL(*pConfigurationProxy, GetInt(ConfigVoice::KEY_EPS_FALLBACK_WATCHDOG_TIME_MILLIS_INT))
            .WillByDefault(Return(6000));
    ON_CALL(*pEpsFbTrigger, IsWaitingRegistration).WillByDefault(Return(IMS_FALSE));

    EXPECT_CALL(objRedialHelper, Redial).Times(0);
    EXPECT_CALL(*pEpsFbTrigger, OnEpsFallbackCompleted).Times(0);
    EXPECT_EQ(
            CallStateName::OUTGOING, pOutgoingState->OnAosStateChanged(MtcAosState::CONNECTED, 0));
}

TEST_F(OutgoingStateTest, HandleAosConnectedNotifiesAndRedialsIfWaitingEpsFallbackForNoResponse)
{
    // Sets up for creating SilentRedialHelper.
    ON_CALL(*pConfigurationProxy,
            GetInt(ConfigVoice::KEY_MO_CALL_REQUEST_TIMEOUT_FOR_EPS_FALLBACK_TRIGGER_MILLIS_INT))
            .WillByDefault(Return(1000));
    ON_CALL(objService, IsNr).WillByDefault(Return(IMS_TRUE));
    ON_CALL(objService, GetSrvccState).WillByDefault(Return(SrvccState::IDLE));
    ON_CALL(objMessageUtils, GetPreviousResponse(&objSession, IMessage::SESSION_START, -1))
            .WillByDefault(Return(IMS_NULL));
    pOutgoingState->SessionStartFailed(&objSession);

    // Test the case.
    ON_CALL(*pConfigurationProxy, GetInt(ConfigVoice::KEY_EPS_FALLBACK_WATCHDOG_TIME_MILLIS_INT))
            .WillByDefault(Return(6000));
    ON_CALL(*pEpsFbTrigger, IsWaitingRegistration).WillByDefault(Return(IMS_TRUE));
    ON_CALL(*pEpsFbTrigger, GetTriggerReason)
            .WillByDefault(Return(EpsFallbackReason::NO_NETWORK_RESPONSE));

    const CallReasonInfo objReasonByEpsfb(CODE_INTERNAL_REDIAL, EXTRA_CODE_REDIAL_BY_EPS_FALLBACK);
    EXPECT_CALL(objController, GetRedialHelper(Ref(objCallContext), objReasonByEpsfb)).Times(0);
    EXPECT_CALL(objRedialHelper, Redial).WillOnce(Return(CallReasonInfo(CODE_NONE)));
    EXPECT_CALL(*pEpsFbTrigger, OnEpsFallbackCompleted);
    EXPECT_EQ(CallStateName::IDLE, pOutgoingState->OnAosStateChanged(MtcAosState::CONNECTED, 0));
}

TEST_F(OutgoingStateTest, HandleAosConnectedNotifiesIfWaitingEpsFallbackForNoTrigger)
{
    ON_CALL(*pConfigurationProxy, GetInt(ConfigVoice::KEY_EPS_FALLBACK_WATCHDOG_TIME_MILLIS_INT))
            .WillByDefault(Return(6000));
    ON_CALL(*pEpsFbTrigger, IsWaitingRegistration).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objController, GetRedialHelper(_, _)).WillByDefault(ReturnRef(objRedialHelper));

    EXPECT_CALL(objRedialHelper, Redial).Times(0);
    EXPECT_CALL(*pEpsFbTrigger, OnEpsFallbackCompleted).Times(0);
    EXPECT_EQ(
            CallStateName::OUTGOING, pOutgoingState->OnAosStateChanged(MtcAosState::CONNECTED, 0));
}

TEST_F(OutgoingStateTest, OnRatChangedPerformsSilentRedialIfWaitingEpsFallback)
{
    // Sets up for creating SilentRedialHelper.
    ON_CALL(*pConfigurationProxy,
            GetInt(ConfigVoice::KEY_MO_CALL_REQUEST_TIMEOUT_FOR_EPS_FALLBACK_TRIGGER_MILLIS_INT))
            .WillByDefault(Return(1000));
    ON_CALL(objService, IsNr).WillByDefault(Return(IMS_TRUE));
    ON_CALL(objService, GetSrvccState).WillByDefault(Return(SrvccState::IDLE));
    ON_CALL(objMessageUtils, GetPreviousResponse(&objSession, IMessage::SESSION_START, -1))
            .WillByDefault(Return(IMS_NULL));
    pOutgoingState->SessionStartFailed(&objSession);

    ON_CALL(*pConfigurationProxy, GetInt(ConfigVoice::KEY_EPS_FALLBACK_WATCHDOG_TIME_MILLIS_INT))
            .WillByDefault(Return(6000));
    ON_CALL(*pEpsFbTrigger, IsWaitingEpsFallback).WillByDefault(Return(IMS_TRUE));
    ON_CALL(objController, GetRedialHelper(_, _)).WillByDefault(ReturnRef(objRedialHelper));

    CallReasonInfo objReason(CODE_NONE);
    EXPECT_CALL(objRedialHelper, Redial).WillOnce(Return(objReason));
    EXPECT_CALL(*pEpsFbTrigger, OnEpsFallbackCompleted);
    EXPECT_EQ(CallStateName::IDLE,
            pOutgoingState->OnRatChanged(
                    INetworkWatcher::RADIOTECH_TYPE_NR, INetworkWatcher::RADIOTECH_TYPE_LTE));
}

TEST_F(OutgoingStateTest, OnReceivingMediaDataStartedStopsUdpKeepAliveSender)
{
    MtcExtensionSet objMtcExtensionSet(GetTestExtensionSet(AString("supportedExtension")));
    ON_CALL(objMtcSession, GetExtensionSet).WillByDefault(ReturnRef(objMtcExtensionSet));

    MockUdpKeepAliveSender* pKeepAliveSender = new MockUdpKeepAliveSender(
            new MockISipKeepAliveHelper(), objCallContext);  // OutgoingState deletes it
    ON_CALL(objCallContext, CreateUdpKeepAliveSender).WillByDefault(Return(pKeepAliveSender));
    ON_CALL(*pConfigurationProxy,
            GetInt(ConfigVoice::KEY_SEND_UDP_KEEP_ALIVE_INTERVAL_TIME_MILLIS_INT))
            .WillByDefault(Return(2000));
    pOutgoingState->SessionProvisionalResponseReceived(&objSession, 0);

    EXPECT_CALL(*pKeepAliveSender, Stop);
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

    ON_CALL(objSession, GetPreviousResponse(IMessage::SESSION_PRACK))
            .WillByDefault(Return(IMS_NULL));
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

    EXPECT_CALL(objMediaManager, AdjustDirectionForLocalResourceConfirmation(_)).Times(0);
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

    EXPECT_CALL(objMediaManager, AdjustDirectionForLocalResourceConfirmation(_)).Times(0);
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

    EXPECT_CALL(objMediaManager, AdjustDirectionForLocalResourceConfirmation(_)).Times(2);
    EXPECT_CALL(objMtcSession, SendEarlyUpdate(UpdateType::NORMAL))
            .Times(2)
            .WillOnce(Return(IMS_FAILURE))
            .WillOnce(Return(IMS_SUCCESS));

    EXPECT_EQ(CallStateName::TERMINATING, pOutgoingState->QosReserved(&objSession, 0));
    EXPECT_EQ(CallStateName::OUTGOING, pOutgoingState->QosReserved(&objSession, 0));
}

TEST_F(OutgoingStateTest, QosReserveFailedTerminatesCallIfNextActionIsReleaseWhenCsfbIsAvailable)
{
    EXPECT_CALL(objMtcSession,
            Terminate(_, CallReasonInfo(CODE_LOCAL_CALL_RESOURCE_RESERVATION_FAILED)));

    ON_CALL(objCallContext, IsCsfbAvailable).WillByDefault(Return(IMS_TRUE));
    EXPECT_CALL(objUiNotifier, SendStartFailed(CallReasonInfo(CODE_LOCAL_CALL_CS_RETRY_REQUIRED)));

    EXPECT_EQ(CallStateName::TERMINATING,
            pOutgoingState->QosReserveFailed(&objSession, QosLossPolicy::RELEASE));
}

TEST_F(OutgoingStateTest, QosReserveFailedTerminatesCallIfNextActionIsReleaseWhenCsfbIsNotAvailable)
{
    EXPECT_CALL(objMtcSession,
            Terminate(_, CallReasonInfo(CODE_LOCAL_CALL_RESOURCE_RESERVATION_FAILED)));

    ON_CALL(objCallContext, IsCsfbAvailable).WillByDefault(Return(IMS_FALSE));
    EXPECT_CALL(objUiNotifier,
            SendStartFailed(CallReasonInfo(CODE_LOCAL_CALL_RESOURCE_RESERVATION_FAILED)));

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

TEST_F(OutgoingStateTest, SessionStartedReturnsOutgoingStateIfNoResponseForUpdate)
{
    MockIMessage objMessage;
    ON_CALL(objMessageUtils, GetPreviousResponse(&objSession, IMessage::SESSION_START, -1))
            .WillByDefault(Return(&objMessage));

    ON_CALL(objMessageUtils, GetResponseStatusCode(&objSession, IMessage::SESSION_EARLY_UPDATE, -1))
            .WillByDefault(Return(SipStatusCode::SC_INVALID));
    MockIMessage objUpdateMessage;
    ON_CALL(objSession, GetPreviousRequest(IMessage::SESSION_EARLY_UPDATE))
            .WillByDefault(Return(&objUpdateMessage));
    ON_CALL(objMessageUtils, HasSdp(&objUpdateMessage)).WillByDefault(Return(IMS_TRUE));

    EXPECT_CALL(objMtcSession, SendAck);

    EXPECT_EQ(CallStateName::OUTGOING, pOutgoingState->SessionStarted(&objSession));
}

TEST_F(OutgoingStateTest, SessionStartedReturnsTerminatingStateIfSendAckFailed)
{
    MockIMessage objMessage;
    ON_CALL(objMessageUtils, GetPreviousResponse(&objSession, IMessage::SESSION_START, -1))
            .WillByDefault(Return(&objMessage));

    ON_CALL(objMessageUtils, GetResponseStatusCode(&objSession, IMessage::SESSION_EARLY_UPDATE, -1))
            .WillByDefault(Return(SipStatusCode::SC_INVALID));
    MockIMessage objUpdateMessage;
    ON_CALL(objSession, GetPreviousRequest(IMessage::SESSION_EARLY_UPDATE))
            .WillByDefault(Return(&objUpdateMessage));
    ON_CALL(objMessageUtils, HasSdp(&objUpdateMessage)).WillByDefault(Return(IMS_TRUE));

    EXPECT_CALL(objMtcSession, SendAck).WillOnce(Return(IMS_FAILURE));

    EXPECT_EQ(CallStateName::TERMINATING, pOutgoingState->SessionStarted(&objSession));
}

TEST_F(OutgoingStateTest, SessionStartedReturnsOutgoingStateIfNoResponseForPrack)
{
    MockIMessage objMessage;
    ON_CALL(objMessageUtils, GetPreviousResponse(&objSession, IMessage::SESSION_START, -1))
            .WillByDefault(Return(&objMessage));

    ON_CALL(objMessageUtils, GetResponseStatusCode(&objSession, IMessage::SESSION_EARLY_UPDATE, -1))
            .WillByDefault(Return(SipStatusCode::SC_INVALID));
    ON_CALL(objSession, GetPreviousRequest(IMessage::SESSION_EARLY_UPDATE))
            .WillByDefault(Return(nullptr));
    MockIMessage objPrackMessage;
    ON_CALL(objSession, GetPreviousRequest(IMessage::SESSION_PRACK))
            .WillByDefault(Return(&objPrackMessage));
    ON_CALL(objMessageUtils, HasSdp(&objPrackMessage)).WillByDefault(Return(IMS_TRUE));

    EXPECT_CALL(objMtcSession, SendAck);

    EXPECT_EQ(CallStateName::OUTGOING, pOutgoingState->SessionStarted(&objSession));
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

TEST_F(OutgoingStateTest, SessionStartedTerminatesCallIfNoCodecMatched)
{
    MockIMessage objMessage;
    ON_CALL(objMessageUtils, GetPreviousResponse(&objSession, IMessage::SESSION_START, -1))
            .WillByDefault(Return(&objMessage));
    const NegotiationResult eNegoResult = NegotiationResult::ERROR_NO_CODEC_MATCHED;
    SetNegotiateSdpFailure(objMessage, eNegoResult);
    ON_CALL(objMessageUtils, GetHeader(_, _, _)).WillByDefault(Return(AString("id")));
    ON_CALL(objMessageUtils, IsHeaderPresent(_, _, _)).WillByDefault(Return(IMS_FALSE));

    EXPECT_CALL(objMtcSession, Terminate(_, CallReasonInfo(CODE_MEDIA_NOT_ACCEPTABLE)));
    EXPECT_CALL(objUiNotifier, SendStartFailed(CallReasonInfo(CODE_MEDIA_NOT_ACCEPTABLE)));

    EXPECT_EQ(CallStateName::TERMINATING, pOutgoingState->SessionStarted(&objSession));
}

TEST_F(OutgoingStateTest, SessionStartedTerminatesCallIfInvalidDescriptor)
{
    MockIMessage objMessage;
    ON_CALL(objMessageUtils, GetPreviousResponse(&objSession, IMessage::SESSION_START, -1))
            .WillByDefault(Return(&objMessage));
    const NegotiationResult eNegoResult = NegotiationResult::ERROR_INVALID_DESCRIPTOR;
    SetNegotiateSdpFailure(objMessage, eNegoResult);
    ON_CALL(objMessageUtils, GetHeader(_, _, _)).WillByDefault(Return(AString("id")));
    ON_CALL(objMessageUtils, IsHeaderPresent(_, _, _)).WillByDefault(Return(IMS_FALSE));

    EXPECT_CALL(objMtcSession, Terminate(_, CallReasonInfo(CODE_REJECT_UNSUPPORTED_SDP_HEADERS)));
    EXPECT_CALL(
            objUiNotifier, SendStartFailed(CallReasonInfo(CODE_REJECT_UNSUPPORTED_SDP_HEADERS)));

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
    EXPECT_CALL(objMtcSession, HandleResponse(ResponseType::ACCEPT, _));

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

    ON_CALL(*pConfigurationProxy, GetInt(ConfigVoice::KEY_EPS_FALLBACK_WATCHDOG_TIME_MILLIS_INT))
            .WillByDefault(Return(2000));
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

TEST_F(OutgoingStateTest, SessionStartedRemovesInactiveSessions)
{
    MockIMtcSession objMtcSession2;
    objSessions.Append(&objMtcSession2);
    ON_CALL(objCallContext, GetSessions()).WillByDefault(ReturnRef(objSessions));

    MockIMessage objMessage;
    ON_CALL(objMessageUtils, GetPreviousResponse(&objSession, IMessage::SESSION_START, -1))
            .WillByDefault(Return(&objMessage));
    ON_CALL(objMessageUtils, IsHeaderPresent(_, _, _)).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMessage, GetStatusCode).WillByDefault(Return(SipStatusCode::SC_200));

    ON_CALL(objMessageUtils, IsResponseExist(&objSession, SipStatusCode::SC_200))
            .WillByDefault(Return(IMS_TRUE));

    EXPECT_CALL(objCallContext, RemoveSession(Ref(objMtcSession))).Times(0);
    EXPECT_CALL(objCallContext, RemoveSession(Ref(objMtcSession2))).Times(1);

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
    ON_CALL(*pConfigurationProxy,
            GetInt(ConfigVoice::KEY_MO_CALL_REQUEST_TIMEOUT_FOR_EPS_FALLBACK_TRIGGER_MILLIS_INT))
            .WillByDefault(Return(1000));
    ON_CALL(objService, IsNr).WillByDefault(Return(IMS_TRUE));
    ON_CALL(objService, GetSrvccState).WillByDefault(Return(SrvccState::IDLE));

    ON_CALL(objMessageUtils, GetPreviousResponse(&objSession, IMessage::SESSION_START, -1))
            .WillByDefault(Return(IMS_NULL));
    SetUpStartErrorHandler(IMS_NULL, SipStatusCode::SC_INVALID, IMS_FALSE,
            ConfigVoice::MO_CALL_REQUEST_TIMEOUT_POLICY_INITIAL_REGISTER_AFTER_CSFB_IF_AVAILBLE,
            IMS_FALSE);

    EXPECT_CALL(*pEpsFbTrigger, TriggerEpsFallback(EpsFallbackReason::NO_NETWORK_RESPONSE));
    EXPECT_EQ(CallStateName::OUTGOING, pOutgoingState->SessionStartFailed(&objSession));
}

TEST_F(OutgoingStateTest, SessionStartFailedInvokesRedialByCallReason)
{
    ON_CALL(objService, GetSrvccState).WillByDefault(Return(SrvccState::IDLE));

    ON_CALL(objMessageUtils, GetPreviousResponse(&objSession, IMessage::SESSION_START, -1))
            .WillByDefault(Return(IMS_NULL));
    SetUpStartErrorHandler(IMS_NULL, SipStatusCode::SC_INVALID, IMS_FALSE,
            ConfigVoice::MO_CALL_REQUEST_TIMEOUT_POLICY_SILENT_REDIAL, IMS_FALSE);

    EXPECT_CALL(objRedialHelper, Redial).Times(1).WillOnce(Return(CallReasonInfo(CODE_NONE)));
    EXPECT_EQ(CallStateName::IDLE, pOutgoingState->SessionStartFailed(&objSession));
}

TEST_F(OutgoingStateTest, SessionStartFailedSetsNetworkResponseTimoutReasonIfSilentRedialFails)
{
    ON_CALL(objService, GetSrvccState).WillByDefault(Return(SrvccState::IDLE));

    ON_CALL(objMessageUtils, GetPreviousResponse(&objSession, IMessage::SESSION_START, -1))
            .WillByDefault(Return(IMS_NULL));
    SetUpStartErrorHandler(IMS_NULL, SipStatusCode::SC_INVALID, IMS_FALSE,
            ConfigVoice::MO_CALL_REQUEST_TIMEOUT_POLICY_SILENT_REDIAL, IMS_FALSE);

    ON_CALL(objRedialHelper, Redial)
            .WillByDefault(
                    Return(CallReasonInfo(CODE_NETWORK_RESP_TIMEOUT, EXTRA_CODE_METHOD_INVITE)));

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
    SetUpStartErrorHandler(&objMessage, SipStatusCode::SC_488, IMS_FALSE, 0, IMS_FALSE);
    SetActionConfigs(ConfigVoice::KEY_REJECT_CODE_AND_ACTION_SET_STRING_ARRAY,
            SipStatusCode::SC_488,
            {ConfigVoice::START_ERROR_ACTION_SILENT_REINVITE_BY_SDP_CONTENT});
    ON_CALL(objMessageUtils, HasSdp(&objMessage)).WillByDefault(Return(IMS_TRUE));

    ON_CALL(objMediaManager, GetSupportedMediaTypesFromSdp(&objSession))
            .WillByDefault(Return(MEDIATYPE_AUDIO));

    ON_CALL(objRedialHelper, Redial)
            .WillByDefault(Return(CallReasonInfo(CODE_SIP_NOT_ACCEPTABLE, SipStatusCode::SC_488)));

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
    SetUpStartErrorHandler(&objMessage, SipStatusCode::SC_301, IMS_FALSE, 0, IMS_FALSE);
    SetActionConfigs(ConfigVoice::KEY_REJECT_CODE_AND_ACTION_SET_STRING_ARRAY,
            SipStatusCode::SC_301, {ConfigVoice::START_ERROR_ACTION_REDIRECTION_BY_CONTACT});
    AString strContactToRedirect("sip:contactToRedirect");
    ON_CALL(objMessageUtils, GetHeaderValue(&objMessage, ISipHeader::CONTACT_NORMAL, _))
            .WillByDefault(Return(strContactToRedirect));

    ON_CALL(objRedialHelper, Redial)
            .WillByDefault(Return(CallReasonInfo(CODE_SIP_REDIRECTED, SipStatusCode::SC_301)));

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
    SetUpStartErrorHandler(&objMessage, SipStatusCode::SC_600, IMS_TRUE, 0, IMS_FALSE);
    SetActionConfigs(ConfigVoice::KEY_REJECT_CODE_AND_ACTION_SET_STRING_ARRAY,
            SipStatusCode::SC_600, {ConfigVoice::START_ERROR_ACTION_CSFB});

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
    SetUpStartErrorHandler(&objMessage, SipStatusCode::SC_380, IMS_FALSE, 0, IMS_FALSE);
    SetActionConfigs(ConfigVoice::KEY_REJECT_CODE_AND_ACTION_SET_STRING_ARRAY,
            SipStatusCode::SC_380,
            {ConfigVoice::START_ERROR_ACTION_NON_UE_DETECTABLE_EMERGENCY_CALL});
    ON_CALL(*pConfigurationProxy,
            GetBoolean(ConfigEmergency::
                            KEY_EMERGENCY_RETRY_WITHOUT_CHECKING_380_CONTENT_FOR_NON_UE_DETECTABLE_EMERGENCY_CALL_BOOL))
            .WillByDefault(Return(IMS_TRUE));

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
        SetUpStartErrorHandler(&objMessage, SipStatusCode::SC_380, IMS_FALSE, 0, IMS_FALSE);
        ON_CALL(*pConfigurationProxy,
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
    objCallInfo.eEmergencyType = EmergencyType::EMERGENCY_ROUTING;
    ON_CALL(objService, GetSrvccState()).WillByDefault(Return(SrvccState::IDLE));

    MockIMessage objMessage;
    ON_CALL(objMessageUtils, GetPreviousResponse(&objSession, IMessage::SESSION_START, -1))
            .WillByDefault(Return(&objMessage));

    ON_CALL(*pConfigurationProxy,
            GetBoolean(ConfigEmergency::
                            KEY_RETRY_EMERGENCY_CALL_OVER_EMERGENCY_PDN_WITH_NEXT_PCSCF_BOOL))
            .WillByDefault(Return(IMS_TRUE));
    ImsVector<AString> objArray;
    ON_CALL(*pConfigurationProxy,
            GetStringArray(
                    ConfigEmergency::KEY_REJECT_CODE_REQUIRE_IMMEDIATE_TERMINATION_STRING_ARRAY))
            .WillByDefault(Return(objArray));
    ON_CALL(*pConfigurationProxy,
            GetInt(ConfigVoice::KEY_SILENT_REDIAL_REGISTRATION_WAIT_TIME_MILLIS_INT))
            .WillByDefault(Return(1000));
    ON_CALL(objService, IsEmergency()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(objMessage, GetStatusCode()).WillByDefault(Return(SipStatusCode::SC_400));
    ON_CALL(objMessageUtils, GetNumberOfPreviousResponses(&objSession, IMessage::SESSION_START))
            .WillByDefault(Return(1));

    EXPECT_CALL(
            objTimer, Start(MtcCallState::TimerType::TIMER_MO_REGISTRATION_FOR_SILENT_REDIAL, _))
            .Times(0);
    EXPECT_CALL(objAosConnector, Control).Times(1);
    EXPECT_EQ(CallStateName::OUTGOING, pOutgoingState->SessionStartFailed(&objSession));

    EXPECT_CALL(objRedialHelper, Redial(_)).Times(1).WillOnce(Return(CallReasonInfo(CODE_NONE)));
    EXPECT_EQ(CallStateName::IDLE, pOutgoingState->OnAosStateChanged(MtcAosState::CONNECTED, 0));
}

TEST_F(OutgoingStateTest, SessionStartFailedIfWaitingForSilentNormalRedial)
{
    ON_CALL(objService, GetSrvccState()).WillByDefault(Return(SrvccState::IDLE));
    ON_CALL(objTimer, IsActive(MtcCallState::TimerType::TIMER_MO_RESPONSE_TIMEOUT_FOR_REASON))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objTimer, IsActive(MtcCallState::TimerType::TIMER_MO_18X_WAIT))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objTimer, IsActive(MtcCallState::TimerType::TIMER_MO_CALL_INITIATION_TO_18X_WAIT))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(*pConfigurationProxy,
            Contains(ConfigVoice::KEY_REGISTRATION_DISCONNECT_REASON_TO_IGNORE_INT_ARRAY,
                    ImsAosReason::REG_NEW_REQUIRED))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(*pConfigurationProxy,
            GetInt(ConfigVoice::KEY_SILENT_REDIAL_REGISTRATION_WAIT_TIME_MILLIS_INT))
            .WillByDefault(Return(1000));

    MockIMessage objMessage;
    ON_CALL(objMessageUtils, GetPreviousResponse(&objSession, IMessage::SESSION_START, -1))
            .WillByDefault(Return(&objMessage));

    ON_CALL(objMessage, GetStatusCode()).WillByDefault(Return(SipStatusCode::SC_503));
    SetActionConfigs(ConfigVoice::KEY_REJECT_CODE_AND_ACTION_SET_STRING_ARRAY,
            SipStatusCode::SC_503, {ConfigVoice::START_ERROR_ACTION_BLOCK_CALL_BY_TIMER});
    ImsList<IMtcCall*> objCalls;
    ON_CALL(objCallManager, GetCallsByState(_)).WillByDefault(Return(objCalls));

    EXPECT_CALL(objTimer, Stop(MtcCallState::TimerType::TIMER_MO_RESPONSE_TIMEOUT_FOR_REASON));
    EXPECT_CALL(objTimer, Stop(MtcCallState::TimerType::TIMER_MO_18X_WAIT));
    EXPECT_CALL(objTimer, Stop(MtcCallState::TimerType::TIMER_MO_CALL_INITIATION_TO_18X_WAIT))
            .Times(0);
    EXPECT_CALL(objTimer,
            Start(MtcCallState::TimerType::TIMER_MO_REGISTRATION_FOR_SILENT_REDIAL, 1000));
    EXPECT_CALL(objAosConnector, RegisterWithNextPcscf(0)).Times(1);
    EXPECT_EQ(CallStateName::OUTGOING, pOutgoingState->SessionStartFailed(&objSession));
    EXPECT_CALL(objMessageUtils, GetPreviousResponse(&objSession, IMessage::SESSION_START, -1))
            .Times(0);
    EXPECT_EQ(CallStateName::OUTGOING, pOutgoingState->SessionStartFailed(&objSession));

    EXPECT_EQ(CallStateName::OUTGOING,
            pOutgoingState->OnAosStateChanged(
                    MtcAosState::DISCONNECTED, ImsAosReason::REG_NEW_REQUIRED));

    EXPECT_CALL(objRedialHelper, Redial(_)).Times(1).WillOnce(Return(CallReasonInfo(CODE_NONE)));
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

TEST_F(OutgoingStateTest, SessionEarlyMediaUpdatedTerminatesCallIfNoCodecMatched)
{
    MockIMessage objMessage;
    ON_CALL(objMessageUtils, GetPreviousResponse(&objSession, IMessage::SESSION_EARLY_UPDATE, -1))
            .WillByDefault(Return(&objMessage));
    const NegotiationResult eNegoResult = NegotiationResult::ERROR_NO_CODEC_MATCHED;
    SetNegotiateSdpFailure(objMessage, eNegoResult);

    EXPECT_CALL(objMtcSession, Terminate(_, CallReasonInfo(CODE_MEDIA_NOT_ACCEPTABLE)));
    EXPECT_CALL(objUiNotifier, SendStartFailed(CallReasonInfo(CODE_MEDIA_NOT_ACCEPTABLE)));

    EXPECT_EQ(CallStateName::TERMINATING, pOutgoingState->SessionEarlyMediaUpdated(&objSession));
}

TEST_F(OutgoingStateTest, SessionEarlyMediaUpdatedTerminatesCallIfInvalidDescriptor)
{
    MockIMessage objMessage;
    ON_CALL(objMessageUtils, GetPreviousResponse(&objSession, IMessage::SESSION_EARLY_UPDATE, -1))
            .WillByDefault(Return(&objMessage));
    const NegotiationResult eNegoResult = NegotiationResult::ERROR_INVALID_DESCRIPTOR;
    SetNegotiateSdpFailure(objMessage, eNegoResult);

    EXPECT_CALL(objMtcSession, Terminate(_, CallReasonInfo(CODE_REJECT_UNSUPPORTED_SDP_HEADERS)));
    EXPECT_CALL(
            objUiNotifier, SendStartFailed(CallReasonInfo(CODE_REJECT_UNSUPPORTED_SDP_HEADERS)));

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

TEST_F(OutgoingStateTest, SessionEarlyMediaUpdatedInvokesSendEarlyUpdate)
{
    MockIMessage objMessage;
    ON_CALL(objMessageUtils, GetPreviousResponse(&objSession, IMessage::SESSION_EARLY_UPDATE, -1))
            .WillByDefault(Return(&objMessage));

    ON_CALL(objPreconditionManager, IsLocalResourceConfirmationRequired(&objSession))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objPreconditionManager, IsAvailableToSendLocalResourceConfirmation(&objSession))
            .WillByDefault(Return(IMS_TRUE));

    ON_CALL(objMediaManager, GetNegotiationState(&objSession))
            .WillByDefault(Return(NegotiationState::STATE_NEGOTIATED));

    EXPECT_CALL(objMediaManager, AdjustDirectionForLocalResourceConfirmation(_)).Times(1);
    EXPECT_CALL(objMtcSession, SendEarlyUpdate(UpdateType::NORMAL)).WillOnce(Return(IMS_SUCCESS));

    EXPECT_EQ(CallStateName::OUTGOING, pOutgoingState->SessionEarlyMediaUpdated(&objSession));
}

TEST_F(OutgoingStateTest, SessionEarlyMediaUpdatedInvokesSessionStartedIfEstablished)
{
    MockIMessage objMessage;
    ON_CALL(objMessageUtils, GetPreviousResponse(&objSession, IMessage::SESSION_EARLY_UPDATE, -1))
            .WillByDefault(Return(&objMessage));

    ON_CALL(objMediaManager, GetNegotiationState(&objSession))
            .WillByDefault(Return(NegotiationState::STATE_NEGOTIATED));

    ON_CALL(objSession, GetState).WillByDefault(Return(ISession::STATE_ESTABLISHED));

    ON_CALL(objMessageUtils, GetPreviousResponse(&objSession, IMessage::SESSION_START, -1))
            .WillByDefault(Return(&objMessage));
    EXPECT_CALL(objUiNotifier, SendStarted);

    EXPECT_EQ(CallStateName::ESTABLISHED, pOutgoingState->SessionEarlyMediaUpdated(&objSession));
}

TEST_F(OutgoingStateTest, SessionEarlyMediaUpdateFailedReturnsTerminating)
{
    ON_CALL(objMessageUtils, GetPreviousResponse(&objSession, IMessage::SESSION_EARLY_UPDATE, -1))
            .WillByDefault(Return(IMS_NULL));

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
    SetActionConfigs(ConfigVoice::KEY_EARLY_UPDATE_REJECT_CODE_AND_ACTION_SET_STRING_ARRAY,
            SipStatusCode::SC_491, {ConfigVoice::EARLY_UPDATE_ERROR_ACTION_GLARE_CONDITION});

    EXPECT_CALL(objMtcSession, Terminate(_, _)).Times(0);
    EXPECT_CALL(objUiNotifier, SendStartFailed(_)).Times(0);
    EXPECT_CALL(objMediaManager, FinalizeSdp(&objSession));
    EXPECT_CALL(objTimer, Start(MtcCallState::TimerType::TIMER_RETRY_UPDATE, _));

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
    SetActionConfigs(ConfigVoice::KEY_EARLY_UPDATE_REJECT_CODE_AND_ACTION_SET_STRING_ARRAY,
            SipStatusCode::SC_503, {ConfigVoice::EARLY_UPDATE_ERROR_ACTION_BLOCK_CALL_BY_TIMER});

    EXPECT_CALL(objMtcSession, Terminate(_, _)).Times(0);
    EXPECT_CALL(objUiNotifier, SendStartFailed(_)).Times(0);
    EXPECT_CALL(objMediaManager, FinalizeSdp(&objSession)).Times(0);
    EXPECT_CALL(objTimer, Start(_, _)).Times(AnyNumber());
    EXPECT_CALL(objTimer, Start(MtcCallState::TimerType::TIMER_RETRY_UPDATE, _)).Times(0);

    EXPECT_EQ(CallStateName::OUTGOING, pOutgoingState->SessionEarlyMediaUpdateFailed(&objSession));

    EXPECT_EQ(CallStateName::OUTGOING,
            pOutgoingState->OnAosStateChanged(
                    MtcAosState::DISCONNECTED, ImsAosReason::REG_NEW_REQUIRED));

    EXPECT_CALL(objRedialHelper, Redial(_)).Times(1).WillOnce(Return(CallReasonInfo(CODE_NONE)));
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
    SetActionConfigs(ConfigVoice::KEY_EARLY_UPDATE_REJECT_CODE_AND_ACTION_SET_STRING_ARRAY,
            SipStatusCode::SC_503, {ConfigVoice::EARLY_UPDATE_ERROR_ACTION_BLOCK_CALL_BY_TIMER});

    EXPECT_CALL(objMtcSession, Terminate(_, _)).Times(0);
    EXPECT_CALL(objUiNotifier, SendStartFailed(_)).Times(0);
    EXPECT_CALL(objMediaManager, FinalizeSdp(&objSession)).Times(0);
    EXPECT_CALL(objTimer, Start(MtcCallState::TimerType::TIMER_RETRY_UPDATE, _)).Times(0);

    IMS_SINT32 nAnyRetryAfter = 10;
    ON_CALL(objMessageUtils, GetHeaderValueInt(&objMessage, ISipHeader::RETRY_AFTER_ANY, _))
            .WillByDefault(Return(nAnyRetryAfter));
    ON_CALL(pConfigService->GetMockCarrierConfig(),
            GetInt(ConfigIms::KEY_SIP_TIMER_F_MILLIS_INT, _))
            .WillByDefault(Return((nAnyRetryAfter + 1) * 1000));
    Engine::GetConfiguration()->RefreshConfigs(objCallContext.GetSlotId());
    ON_CALL(objCallContext, IsCsfbAvailable).WillByDefault(Return(IMS_FALSE));

    EXPECT_CALL(objRedialHelper, Redial(_)).Times(1).WillOnce(Return(CallReasonInfo(CODE_NONE)));
    EXPECT_EQ(CallStateName::IDLE, pOutgoingState->SessionEarlyMediaUpdateFailed(&objSession));
}

TEST_F(OutgoingStateTest, SessionEarlyMediaUpdateFailedTerminatesSessionAndKeepsCurrentState)
{
    MockIMtcSession objMtcSession2;
    objSessions.Append(&objMtcSession2);
    ON_CALL(objCallContext, GetSessions()).WillByDefault(ReturnRef(objSessions));

    MockIMessage objMessage;
    ON_CALL(objMessage, GetStatusCode).WillByDefault(Return(SipStatusCode::SC_481));
    ON_CALL(objMessageUtils, GetPreviousResponse(&objSession, IMessage::SESSION_EARLY_UPDATE, _))
            .WillByDefault(Return(&objMessage));
    SetActionConfigs(ConfigVoice::KEY_EARLY_UPDATE_REJECT_CODE_AND_ACTION_SET_STRING_ARRAY,
            SipStatusCode::SC_481, {ConfigVoice::EARLY_UPDATE_ERROR_ACTION_TERMINATE_DIALOG});

    EXPECT_CALL(objMtcSession, Terminate(IMS_TRUE, CallReasonInfo(CODE_SIP_SERVER_ERROR)));
    EXPECT_CALL(objUiNotifier, SendStartFailed(_)).Times(0);
    EXPECT_CALL(objCallContext, RemoveSession(Ref(objMtcSession))).Times(1);
    EXPECT_CALL(objCallContext, RemoveSession(Ref(objMtcSession2))).Times(0);

    EXPECT_EQ(CallStateName::OUTGOING, pOutgoingState->SessionEarlyMediaUpdateFailed(&objSession));
}

TEST_F(OutgoingStateTest, SessionEarlyMediaUpdateFailedTerminatesCallWhenOnlyOneDialogExists)
{
    MockIMessage objMessage;
    ON_CALL(objMessage, GetStatusCode).WillByDefault(Return(SipStatusCode::SC_481));
    ON_CALL(objMessageUtils, GetPreviousResponse(&objSession, IMessage::SESSION_EARLY_UPDATE, _))
            .WillByDefault(Return(&objMessage));
    SetActionConfigs(ConfigVoice::KEY_EARLY_UPDATE_REJECT_CODE_AND_ACTION_SET_STRING_ARRAY,
            SipStatusCode::SC_481, {ConfigVoice::EARLY_UPDATE_ERROR_ACTION_TERMINATE_DIALOG});

    EXPECT_CALL(objMtcSession, Terminate(IMS_FALSE, CallReasonInfo(CODE_REJECT_INTERNAL_ERROR)));
    EXPECT_CALL(objUiNotifier, SendStartFailed(_)).Times(1);

    EXPECT_EQ(
            CallStateName::TERMINATING, pOutgoingState->SessionEarlyMediaUpdateFailed(&objSession));
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

TEST_F(OutgoingStateTest, SessionForkedResponseReceivedAddsISession)
{
    ON_CALL(*pConfigurationProxy,
            GetBoolean(ConfigVoice::KEY_MAINTAIN_MULTIPLE_EARLY_SESSIONS_BY_FORKING_BOOL))
            .WillByDefault(Return(IMS_TRUE));
    EXPECT_CALL(objMtcSession,
            Terminate(IMS_TRUE, CallReasonInfo(CODE_INTERNAL_EARLYDIALOG_FORKED_TERMINATED)))
            .Times(0);
    EXPECT_CALL(objCallContext, RemoveSession(Ref(objMtcSession))).Times(0);

    MockISession objForkedSession;
    EXPECT_CALL(*pSessionInterfaceHolder, AddISession(_, &objForkedSession));
    EXPECT_EQ(CallStateName::OUTGOING,
            pOutgoingState->SessionForkedResponseReceived(&objSession, &objForkedSession));
}

TEST_F(OutgoingStateTest, SessionForkedResponseReceivedTerminatesOriginalSessionByConfig)
{
    ON_CALL(*pConfigurationProxy,
            GetBoolean(ConfigVoice::KEY_MAINTAIN_MULTIPLE_EARLY_SESSIONS_BY_FORKING_BOOL))
            .WillByDefault(Return(IMS_FALSE));
    EXPECT_CALL(objMtcSession,
            Terminate(IMS_TRUE, CallReasonInfo(CODE_INTERNAL_EARLYDIALOG_FORKED_TERMINATED)))
            .Times(1);
    EXPECT_CALL(objCallContext, RemoveSession(Ref(objMtcSession))).Times(1);

    MockISession objForkedSession;
    EXPECT_EQ(CallStateName::OUTGOING,
            pOutgoingState->SessionForkedResponseReceived(&objSession, &objForkedSession));
}

TEST_F(OutgoingStateTest, SessionForkedResponseReceivedDoesNotTerminateOriginalSessionIfNotFound)
{
    ON_CALL(*pConfigurationProxy,
            GetBoolean(ConfigVoice::KEY_MAINTAIN_MULTIPLE_EARLY_SESSIONS_BY_FORKING_BOOL))
            .WillByDefault(Return(IMS_FALSE));

    ON_CALL(objCallContext, GetSession(_)).WillByDefault(Return(nullptr));

    EXPECT_CALL(objMtcSession,
            Terminate(IMS_TRUE, CallReasonInfo(CODE_INTERNAL_EARLYDIALOG_FORKED_TERMINATED)))
            .Times(0);
    EXPECT_CALL(objCallContext, RemoveSession(Ref(objMtcSession))).Times(0);

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
    EXPECT_CALL(objMediaManager, AdjustDirectionForLocalResourceConfirmation(_)).Times(0);
    EXPECT_EQ(CallStateName::OUTGOING, pOutgoingState->SessionPrackDelivered(&objSession));
}

TEST_F(OutgoingStateTest, SessionPrackDeliveredReturnsOutgoingIfNotAvailableToSendEarlyUpdate)
{
    MockIMessage objMessage;
    ON_CALL(objSession, GetPreviousResponse(IMessage::SESSION_PRACK))
            .WillByDefault(Return(&objMessage));
    ON_CALL(objMessageUtils, GetResponseStatusCode(&objSession, IMessage::SESSION_START, -1))
            .WillByDefault(Return(SipStatusCode::SC_183));
    ON_CALL(objPreconditionManager, IsLocalResourceConfirmationRequired(&objSession))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objPreconditionManager, IsAvailableToSendLocalResourceConfirmation(&objSession))
            .WillByDefault(Return(IMS_FALSE));

    EXPECT_CALL(objMediaManager, AdjustDirectionForLocalResourceConfirmation(_)).Times(0);
    EXPECT_EQ(CallStateName::OUTGOING, pOutgoingState->SessionPrackDelivered(&objSession));
}

TEST_F(OutgoingStateTest, SessionPrackDeliveredReturnsOutgoingIfCodeIs183ButNotNegotiated)
{
    MockIMessage objMessage;
    ON_CALL(objSession, GetPreviousResponse(IMessage::SESSION_PRACK))
            .WillByDefault(Return(&objMessage));
    MockISipMessage objSipMessage;
    ON_CALL(objMessage, GetMessage).WillByDefault(Return(&objSipMessage));
    SetSdpOaSuccessWithSdp(objMessage, objSipMessage);  // to make IMessage have SDP

    ON_CALL(objPreconditionManager, IsLocalResourceConfirmationRequired(&objSession))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objPreconditionManager, IsAvailableToSendLocalResourceConfirmation(&objSession))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objMessageUtils, GetResponseStatusCode(&objSession, IMessage::SESSION_START, -1))
            .WillByDefault(Return(SipStatusCode::SC_183));
    ON_CALL(objMediaManager, GetNegotiationState(&objSession))
            .WillByDefault(Return(NegotiationState::STATE_OFFER_SENT));

    EXPECT_CALL(objMediaManager, AdjustDirectionForLocalResourceConfirmation(_)).Times(0);
    EXPECT_EQ(CallStateName::OUTGOING, pOutgoingState->SessionPrackDelivered(&objSession));
}

TEST_F(OutgoingStateTest, SessionPrackDeliveredReturnsTerminatingIfSdpOaFails)
{
    MockIMessage objMessage;
    ON_CALL(objSession, GetPreviousResponse(IMessage::SESSION_PRACK))
            .WillByDefault(Return(&objMessage));
    SetSdpOaFailure(objMessage);

    EXPECT_CALL(objMtcSession, Terminate(_, CallReasonInfo(CODE_MEDIA_NOT_ACCEPTABLE)));
    EXPECT_CALL(objUiNotifier, SendStartFailed(CallReasonInfo(CODE_MEDIA_NOT_ACCEPTABLE)));
    EXPECT_EQ(CallStateName::TERMINATING, pOutgoingState->SessionPrackDelivered(&objSession));
}

TEST_F(OutgoingStateTest, SessionPrackDeliveredReturnsTerminatingIfNoCodecMatched)
{
    MockIMessage objMessage;
    ON_CALL(objSession, GetPreviousResponse(IMessage::SESSION_PRACK))
            .WillByDefault(Return(&objMessage));
    const NegotiationResult eNegoResult = NegotiationResult::ERROR_NO_CODEC_MATCHED;
    SetNegotiateSdpFailure(objMessage, eNegoResult);

    EXPECT_CALL(objMtcSession, Terminate(_, CallReasonInfo(CODE_MEDIA_NOT_ACCEPTABLE)));
    EXPECT_CALL(objUiNotifier, SendStartFailed(CallReasonInfo(CODE_MEDIA_NOT_ACCEPTABLE)));
    EXPECT_EQ(CallStateName::TERMINATING, pOutgoingState->SessionPrackDelivered(&objSession));
}

TEST_F(OutgoingStateTest, SessionPrackDeliveredReturnsTerminatingIfInvalidDescriptor)
{
    MockIMessage objMessage;
    ON_CALL(objSession, GetPreviousResponse(IMessage::SESSION_PRACK))
            .WillByDefault(Return(&objMessage));
    const NegotiationResult eNegoResult = NegotiationResult::ERROR_INVALID_DESCRIPTOR;
    SetNegotiateSdpFailure(objMessage, eNegoResult);

    EXPECT_CALL(objMtcSession, Terminate(_, CallReasonInfo(CODE_REJECT_UNSUPPORTED_SDP_HEADERS)));
    EXPECT_CALL(
            objUiNotifier, SendStartFailed(CallReasonInfo(CODE_REJECT_UNSUPPORTED_SDP_HEADERS)));
    EXPECT_EQ(CallStateName::TERMINATING, pOutgoingState->SessionPrackDelivered(&objSession));
}

TEST_F(OutgoingStateTest, SessionPrackDeliveredReturnsOutgoingIfNoSdpOa)
{
    MockIMessage objMessage;
    ON_CALL(objSession, GetPreviousResponse(IMessage::SESSION_PRACK))
            .WillByDefault(Return(&objMessage));
    ON_CALL(objMessageUtils, HasSdp(&objMessage)).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMediaManager, GetNegotiationState(&objSession))
            .WillByDefault(Return(NegotiationState::STATE_OFFER_SENT));
    SipMethod objPrackMethod = SipMethod::PRACK;
    ON_CALL(objMessage, GetMethod).WillByDefault(ReturnRef(objPrackMethod));

    ON_CALL(objPreconditionManager, IsLocalResourceConfirmationRequired(&objSession))
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(objPreconditionManager, IsAvailableToSendLocalResourceConfirmation(&objSession))
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMediaManager, GetNegotiationState(&objSession))
            .WillByDefault(Return(NegotiationState::STATE_OFFER_SENT));

    MockIMessage objPreviousPrackMessage;
    ON_CALL(objSession, GetPreviousRequest(IMessage::SESSION_PRACK))
            .WillByDefault(Return(&objPreviousPrackMessage));
    ON_CALL(objMessageUtils, HasSdp(&objPreviousPrackMessage)).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objPreviousPrackMessage, GetState()).WillByDefault(Return(IMessage::STATE_SENT));

    EXPECT_CALL(objMtcSession, Terminate(_, _)).Times(0);
    EXPECT_CALL(objUiNotifier, SendStartFailed(_)).Times(0);
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

    EXPECT_CALL(objMediaManager, AdjustDirectionForLocalResourceConfirmation(_)).Times(1);
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

    EXPECT_CALL(objMediaManager, AdjustDirectionForLocalResourceConfirmation(_)).Times(1);
    EXPECT_CALL(objMtcSession, SendEarlyUpdate(UpdateType::NORMAL)).WillOnce(Return(IMS_FAILURE));

    EXPECT_CALL(objMtcSession, Terminate(_, CallReasonInfo(CODE_REJECT_INTERNAL_ERROR)));
    EXPECT_CALL(objUiNotifier, SendStartFailed(CallReasonInfo(CODE_REJECT_INTERNAL_ERROR)));

    EXPECT_EQ(CallStateName::TERMINATING, pOutgoingState->SessionPrackDelivered(&objSession));
}

TEST_F(OutgoingStateTest, SessionPrackDeliveredReturnsOutgoingIf200OkIsAlreadyReceivedAndNoSdp)
{
    MockIMessage objMessage;
    ON_CALL(objSession, GetPreviousResponse(IMessage::SESSION_PRACK))
            .WillByDefault(Return(&objMessage));

    ON_CALL(objMessageUtils, GetResponseStatusCode(&objSession, IMessage::SESSION_START, -1))
            .WillByDefault(Return(SipStatusCode::SC_200));

    EXPECT_CALL(objMediaManager, AdjustDirectionForLocalResourceConfirmation(_)).Times(0);
    EXPECT_CALL(objMtcSession, SendEarlyUpdate(UpdateType::NORMAL)).Times(0);

    EXPECT_EQ(CallStateName::OUTGOING, pOutgoingState->SessionPrackDelivered(&objSession));
}

TEST_F(OutgoingStateTest, SessionPrackDeliveredInvokesSessionStartedIfEstablished)
{
    MockIMessage objMessage;
    ON_CALL(objSession, GetPreviousResponse(IMessage::SESSION_PRACK))
            .WillByDefault(Return(&objMessage));

    ON_CALL(objMessageUtils, GetResponseStatusCode(&objSession, IMessage::SESSION_START, -1))
            .WillByDefault(Return(SipStatusCode::SC_200));
    ON_CALL(objMediaManager, GetNegotiationState(&objSession))
            .WillByDefault(Return(NegotiationState::STATE_NEGOTIATED));

    MockISipMessage objSipMessage;
    ON_CALL(objMessage, GetMessage).WillByDefault(Return(&objSipMessage));
    SetSdpOaSuccessWithSdp(objMessage, objSipMessage);  // to make IMessage have SDP

    ON_CALL(objMessageUtils, GetPreviousResponse(&objSession, IMessage::SESSION_START, -1))
            .WillByDefault(Return(&objMessage));

    EXPECT_CALL(objUiNotifier, SendStarted);
    EXPECT_EQ(CallStateName::ESTABLISHED, pOutgoingState->SessionPrackDelivered(&objSession));
}

TEST_F(OutgoingStateTest, SessionPrackDeliveryFailedDoesNothingIfNeedToIgnoreByConfig)
{
    ON_CALL(*pConfigurationProxy, GetInt(ConfigVoice::KEY_POLICY_FOR_PRACK_DELIVERY_FAILURE_INT))
            .WillByDefault(Return(ConfigVoice::PRACK_DELIVERY_FAILURE_POLICY_IGNORE));

    EXPECT_EQ(CallStateName::OUTGOING, pOutgoingState->SessionPrackDeliveryFailed(&objSession));
}

TEST_F(OutgoingStateTest, SessionPrackDeliveryFailedByErrorResponseTerminatesCall)
{
    ON_CALL(*pConfigurationProxy, GetInt(ConfigVoice::KEY_POLICY_FOR_PRACK_DELIVERY_FAILURE_INT))
            .WillByDefault(Return(ConfigVoice::PRACK_DELIVERY_FAILURE_POLICY_TERMINATE_DIALOG));
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
    ON_CALL(*pConfigurationProxy, GetInt(ConfigVoice::KEY_POLICY_FOR_PRACK_DELIVERY_FAILURE_INT))
            .WillByDefault(Return(ConfigVoice::PRACK_DELIVERY_FAILURE_POLICY_TERMINATE_DIALOG));
    ON_CALL(objMessageUtils, GetResponseStatusCode(&objSession, IMessage::SESSION_PRACK, -1))
            .WillByDefault(Return(SipStatusCode::SC_INVALID));

    EXPECT_CALL(objMtcSession,
            Terminate(_, CallReasonInfo(CODE_NETWORK_RESP_TIMEOUT, EXTRA_CODE_METHOD_PRACK)));
    EXPECT_CALL(objUiNotifier,
            SendStartFailed(CallReasonInfo(CODE_NETWORK_RESP_TIMEOUT, EXTRA_CODE_METHOD_PRACK)));

    EXPECT_EQ(CallStateName::TERMINATING, pOutgoingState->SessionPrackDeliveryFailed(&objSession));
}

TEST_F(OutgoingStateTest, SessionPrackDeliveryFailedTerminatesSessionAndKeepsCurrentState)
{
    ON_CALL(*pConfigurationProxy, GetInt(ConfigVoice::KEY_POLICY_FOR_PRACK_DELIVERY_FAILURE_INT))
            .WillByDefault(Return(ConfigVoice::PRACK_DELIVERY_FAILURE_POLICY_TERMINATE_DIALOG));

    MockIMtcSession objMtcSession2;
    objSessions.Append(&objMtcSession2);
    ON_CALL(objCallContext, GetSessions()).WillByDefault(ReturnRef(objSessions));

    EXPECT_CALL(objMtcSession, Terminate(IMS_TRUE, CallReasonInfo(CODE_SIP_SERVER_ERROR)));
    EXPECT_CALL(objUiNotifier, SendStartFailed(_)).Times(0);
    EXPECT_CALL(objCallContext, RemoveSession(Ref(objMtcSession))).Times(1);
    EXPECT_CALL(objCallContext, RemoveSession(Ref(objMtcSession2))).Times(0);

    EXPECT_EQ(CallStateName::OUTGOING, pOutgoingState->SessionPrackDeliveryFailed(&objSession));
}

TEST_F(OutgoingStateTest, SessionPrackDeliveryFailedTerminatesCallOnMultiDialog)
{
    ON_CALL(*pConfigurationProxy, GetInt(ConfigVoice::KEY_POLICY_FOR_PRACK_DELIVERY_FAILURE_INT))
            .WillByDefault(Return(ConfigVoice::PRACK_DELIVERY_FAILURE_POLICY_TERMINATE_CALL));
    ON_CALL(objMessageUtils, GetResponseStatusCode(&objSession, IMessage::SESSION_PRACK, -1))
            .WillByDefault(Return(SipStatusCode::SC_504));

    MockIMtcSession objMtcSession2;
    objSessions.Append(&objMtcSession2);
    ON_CALL(objCallContext, GetSessions()).WillByDefault(ReturnRef(objSessions));

    EXPECT_CALL(objMtcSession,
            Terminate(_, CallReasonInfo(CODE_SIP_METHOD_NOT_ALLOWED, EXTRA_CODE_METHOD_PRACK)));
    EXPECT_CALL(objUiNotifier,
            SendStartFailed(CallReasonInfo(CODE_SIP_METHOD_NOT_ALLOWED, EXTRA_CODE_METHOD_PRACK)));

    EXPECT_EQ(CallStateName::TERMINATING, pOutgoingState->SessionPrackDeliveryFailed(&objSession));
}

TEST_F(OutgoingStateTest, SessionProvisionalResponseReceivedStopsTimersIfNot100)
{
    MtcExtensionSet objMtcExtensionSet(GetTestExtensionSet(AString("supportedExtension")));
    ON_CALL(objMtcSession, GetExtensionSet).WillByDefault(ReturnRef(objMtcExtensionSet));

    ON_CALL(objMessageUtils, GetResponseStatusCode(&objSession, IMessage::SESSION_START, 0))
            .WillByDefault(Return(SipStatusCode::SC_183));
    ON_CALL(objTimer, IsActive(MtcCallState::TimerType::TIMER_MO_RESPONSE_TIMEOUT_FOR_REASON))
            .WillByDefault(Return(IMS_TRUE));
    EXPECT_CALL(objTimer, Stop(MtcCallState::TimerType::TIMER_MO_RESPONSE_TIMEOUT_FOR_REASON));

    ON_CALL(objTimer, IsActive(MtcCallState::TimerType::TIMER_MO_18X_WAIT))
            .WillByDefault(Return(IMS_TRUE));
    EXPECT_CALL(objTimer, Stop(MtcCallState::TimerType::TIMER_MO_18X_WAIT));
    ON_CALL(objTimer, IsActive(MtcCallState::TimerType::TIMER_MO_CALL_INITIATION_TO_18X_WAIT))
            .WillByDefault(Return(IMS_TRUE));
    EXPECT_CALL(objTimer, Stop(MtcCallState::TimerType::TIMER_MO_CALL_INITIATION_TO_18X_WAIT));

    EXPECT_CALL(objPassiveTimer, RemoveTimer(IPassiveTimerHolder::Type::REGISTRATION_TO_18X));

    pOutgoingState->SessionProvisionalResponseReceived(&objSession, 0);
}

TEST_F(OutgoingStateTest, SessionProvisionalResponseReceivedDoesNotStart18xWaitTimerIfNot100)
{
    MtcExtensionSet objMtcExtensionSet(GetTestExtensionSet(AString("supportedExtension")));
    ON_CALL(objMtcSession, GetExtensionSet).WillByDefault(ReturnRef(objMtcExtensionSet));

    ON_CALL(objMessageUtils, GetResponseStatusCode(&objSession, IMessage::SESSION_START, 0))
            .WillByDefault(Return(SipStatusCode::SC_183));

    EXPECT_CALL(objTimer, Start(MtcCallState::TimerType::TIMER_MO_18X_WAIT, _)).Times(0);
    EXPECT_CALL(objTimer, Start(MtcCallState::TimerType::TIMER_MO_NOANSWER, _));

    pOutgoingState->SessionProvisionalResponseReceived(&objSession, 0);
}

TEST_F(OutgoingStateTest, SessionProvisionalResponseReceivedStops100WaitTimerIf100)
{
    MtcExtensionSet objMtcExtensionSet(GetTestExtensionSet(AString("supportedExtension")));
    ON_CALL(objMtcSession, GetExtensionSet).WillByDefault(ReturnRef(objMtcExtensionSet));

    ON_CALL(objMessageUtils, GetResponseStatusCode(&objSession, IMessage::SESSION_START, 0))
            .WillByDefault(Return(SipStatusCode::SC_100));
    ON_CALL(objTimer, IsActive(MtcCallState::TimerType::TIMER_MO_RESPONSE_TIMEOUT_FOR_REASON))
            .WillByDefault(Return(IMS_TRUE));
    EXPECT_CALL(objTimer, Stop(MtcCallState::TimerType::TIMER_MO_RESPONSE_TIMEOUT_FOR_REASON));

    ON_CALL(objTimer, IsActive(MtcCallState::TimerType::TIMER_MO_18X_WAIT))
            .WillByDefault(Return(IMS_TRUE));
    EXPECT_CALL(objTimer, Stop(MtcCallState::TimerType::TIMER_MO_18X_WAIT)).Times(0);

    EXPECT_CALL(objPassiveTimer, RemoveTimer(IPassiveTimerHolder::Type::REGISTRATION_TO_18X))
            .Times(0);

    pOutgoingState->SessionProvisionalResponseReceived(&objSession, 0);
}

TEST_F(OutgoingStateTest, SessionProvisionalResponseReceivedStarts18xWaitTimerIf100)
{
    MtcExtensionSet objMtcExtensionSet(GetTestExtensionSet(AString("supportedExtension")));
    ON_CALL(objMtcSession, GetExtensionSet).WillByDefault(ReturnRef(objMtcExtensionSet));

    ON_CALL(objMessageUtils, GetResponseStatusCode(&objSession, IMessage::SESSION_START, 0))
            .WillByDefault(Return(SipStatusCode::SC_100));

    EXPECT_CALL(objTimer, Start(MtcCallState::TimerType::TIMER_MO_18X_WAIT, _));

    pOutgoingState->SessionProvisionalResponseReceived(&objSession, 0);
}

TEST_F(OutgoingStateTest, SessionProvisionalResponseReceivedStartsUdpKeepAliveSenderIfSupported)
{
    MtcExtensionSet objMtcExtensionSet(GetTestExtensionSet(AString("supportedExtension")));
    ON_CALL(objMtcSession, GetExtensionSet).WillByDefault(ReturnRef(objMtcExtensionSet));

    MockUdpKeepAliveSender* pKeepAliveSender = new MockUdpKeepAliveSender(
            new MockISipKeepAliveHelper(), objCallContext);  // OutgoingState deletes it
    ON_CALL(objCallContext, CreateUdpKeepAliveSender).WillByDefault(Return(pKeepAliveSender));
    ON_CALL(*pConfigurationProxy,
            GetInt(ConfigVoice::KEY_SEND_UDP_KEEP_ALIVE_INTERVAL_TIME_MILLIS_INT))
            .WillByDefault(Return(2000));
    EXPECT_CALL(*pKeepAliveSender, Start).Times(1);

    pOutgoingState->SessionProvisionalResponseReceived(&objSession, 0);
    pOutgoingState->SessionProvisionalResponseReceived(&objSession, 1);
}

TEST_F(OutgoingStateTest,
        SessionProvisionalResponseReceivedTerminatesCallIfRequiredExtensionIsNotSupported)
{
    MtcExtensionSet objMtcExtensionSet(GetTestExtensionSet(AString("supportedExtension")));
    ON_CALL(objMtcSession, GetExtensionSet).WillByDefault(ReturnRef(objMtcExtensionSet));

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
    MtcExtensionSet objMtcExtensionSet(GetTestExtensionSet(AString("supportedExtension")));
    ON_CALL(objMtcSession, GetExtensionSet).WillByDefault(ReturnRef(objMtcExtensionSet));
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
    MtcExtensionSet objMtcExtensionSet(GetTestExtensionSet(AString("supportedExtension")));
    ON_CALL(objMtcSession, GetExtensionSet).WillByDefault(ReturnRef(objMtcExtensionSet));
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

TEST_F(OutgoingStateTest, SessionProvisionalResponseReceivedTerminatesCallIfNoCodecMatched)
{
    MtcExtensionSet objMtcExtensionSet(GetTestExtensionSet(AString("supportedExtension")));
    ON_CALL(objMtcSession, GetExtensionSet).WillByDefault(ReturnRef(objMtcExtensionSet));
    ON_CALL(objMessageUtils, GetResponseStatusCode(&objSession, IMessage::SESSION_START, 0))
            .WillByDefault(Return(SipStatusCode::SC_180));
    MockIMessage objMessage;
    ON_CALL(objMessageUtils, GetPreviousResponse(&objSession, IMessage::SESSION_START, 0))
            .WillByDefault(Return(&objMessage));
    EXPECT_CALL(objMediaManager, UpdatePemType(&objSession, &objMessage));

    const NegotiationResult eNegoResult = NegotiationResult::ERROR_NO_CODEC_MATCHED;
    SetNegotiateSdpFailure(objMessage, eNegoResult);

    EXPECT_CALL(objMtcSession, Terminate(_, CallReasonInfo(CODE_MEDIA_NOT_ACCEPTABLE)));
    EXPECT_CALL(objUiNotifier, SendStartFailed(CallReasonInfo(CODE_MEDIA_NOT_ACCEPTABLE)));
    EXPECT_EQ(CallStateName::TERMINATING,
            pOutgoingState->SessionProvisionalResponseReceived(&objSession, 0));
}

TEST_F(OutgoingStateTest, SessionProvisionalResponseReceivedTerminatesCallIfInvalidDescriptor)
{
    MtcExtensionSet objMtcExtensionSet(GetTestExtensionSet(AString("supportedExtension")));
    ON_CALL(objMtcSession, GetExtensionSet).WillByDefault(ReturnRef(objMtcExtensionSet));
    ON_CALL(objMessageUtils, GetResponseStatusCode(&objSession, IMessage::SESSION_START, 0))
            .WillByDefault(Return(SipStatusCode::SC_180));
    MockIMessage objMessage;
    ON_CALL(objMessageUtils, GetPreviousResponse(&objSession, IMessage::SESSION_START, 0))
            .WillByDefault(Return(&objMessage));
    EXPECT_CALL(objMediaManager, UpdatePemType(&objSession, &objMessage));

    const NegotiationResult eNegoResult = NegotiationResult::ERROR_INVALID_DESCRIPTOR;
    SetNegotiateSdpFailure(objMessage, eNegoResult);

    EXPECT_CALL(objMtcSession, Terminate(_, CallReasonInfo(CODE_REJECT_UNSUPPORTED_SDP_HEADERS)));
    EXPECT_CALL(
            objUiNotifier, SendStartFailed(CallReasonInfo(CODE_REJECT_UNSUPPORTED_SDP_HEADERS)));
    EXPECT_EQ(CallStateName::TERMINATING,
            pOutgoingState->SessionProvisionalResponseReceived(&objSession, 0));
}

TEST_F(OutgoingStateTest, SessionProvisionalResponseReceivedInvokesSendProgressing)
{
    MtcExtensionSet objMtcExtensionSet(GetTestExtensionSet(AString("supportedExtension")));
    ON_CALL(objMtcSession, GetExtensionSet).WillByDefault(ReturnRef(objMtcExtensionSet));
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
    MtcExtensionSet objMtcExtensionSet(GetTestExtensionSet(AString("supportedExtension")));
    ON_CALL(objMtcSession, GetExtensionSet).WillByDefault(ReturnRef(objMtcExtensionSet));

    MockIMessage objMessage;
    ON_CALL(objMessageUtils, GetPreviousResponse(&objSession, IMessage::SESSION_START, 0))
            .WillByDefault(Return(&objMessage));

    ON_CALL(objTimer, IsActive(MtcCallState::TimerType::TIMER_MO_RESPONSE_TIMEOUT_FOR_REASON))
            .WillByDefault(Return(IMS_TRUE));
    EXPECT_CALL(objTimer, Stop(MtcCallState::TimerType::TIMER_MO_RESPONSE_TIMEOUT_FOR_REASON));

    ON_CALL(objTimer, IsActive(MtcCallState::TimerType::TIMER_MO_18X_WAIT))
            .WillByDefault(Return(IMS_TRUE));
    EXPECT_CALL(objTimer, Stop(MtcCallState::TimerType::TIMER_MO_18X_WAIT));
    ON_CALL(objTimer, IsActive(MtcCallState::TimerType::TIMER_MO_CALL_INITIATION_TO_18X_WAIT))
            .WillByDefault(Return(IMS_TRUE));
    EXPECT_CALL(objTimer, Stop(MtcCallState::TimerType::TIMER_MO_CALL_INITIATION_TO_18X_WAIT));

    EXPECT_CALL(objPassiveTimer, RemoveTimer(IPassiveTimerHolder::Type::REGISTRATION_TO_18X));

    pOutgoingState->SessionRprReceived(&objSession, 0);
}

TEST_F(OutgoingStateTest, SessionRprReceivedStopOrStartMoNoanswerTimerByContext)
{
    MtcExtensionSet objMtcExtensionSet(GetTestExtensionSet(AString("supportedExtension")));
    ON_CALL(objMtcSession, GetExtensionSet).WillByDefault(ReturnRef(objMtcExtensionSet));

    MockIMessage objMessage;
    ON_CALL(objMessageUtils, GetPreviousResponse(&objSession, IMessage::SESSION_START, 0))
            .WillByDefault(Return(&objMessage));

    // config is true
    ON_CALL(*pConfigurationProxy,
            GetBoolean(ConfigVoice::KEY_STOP_RINGBACK_TIMER_BY_183_WITH_SDP_BODY_BOOL))
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
    ON_CALL(*pConfigurationProxy,
            GetBoolean(ConfigVoice::KEY_STOP_RINGBACK_TIMER_BY_183_WITH_SDP_BODY_BOOL))
            .WillByDefault(Return(IMS_FALSE));

    EXPECT_CALL(objTimer, Start(MtcCallState::TimerType::TIMER_MO_NOANSWER, _));

    pOutgoingState->SessionRprReceived(&objSession, 0);
}

TEST_F(OutgoingStateTest, SessionRprReceivedTerminatesCallIfRequiredExtensionIsNotSupported)
{
    MtcExtensionSet objMtcExtensionSet(GetTestExtensionSet(AString("supportedExtension")));
    ON_CALL(objMtcSession, GetExtensionSet).WillByDefault(ReturnRef(objMtcExtensionSet));

    MockIMessage objMessage;
    ON_CALL(objMessageUtils, GetPreviousResponse(&objSession, IMessage::SESSION_START, 0))
            .WillByDefault(Return(&objMessage));

    const AString strUnsupportedExtension("unsupportedExtension");
    ImsList<AString> lstRequiredExtensions;
    lstRequiredExtensions.Append(strUnsupportedExtension);
    ON_CALL(objMessageUtils, GetHeaders(&objMessage, ISipHeader::REQUIRE, _))
            .WillByDefault(Return(lstRequiredExtensions));

    EXPECT_CALL(objMtcSession, SendPrack(IMS_FALSE));
    EXPECT_CALL(objMtcSession, Terminate(_, CallReasonInfo(CODE_LOCAL_SERVICE_UNAVAILABLE)));
    EXPECT_CALL(objUiNotifier, SendStartFailed(CallReasonInfo(CODE_LOCAL_SERVICE_UNAVAILABLE)));

    EXPECT_EQ(CallStateName::TERMINATING, pOutgoingState->SessionRprReceived(&objSession, 0));
}

TEST_F(OutgoingStateTest, SessionRprReceivedReturnsOutgoingStateIf199)
{
    MtcExtensionSet objMtcExtensionSet(GetTestExtensionSet(AString("supportedExtension")));
    ON_CALL(objMtcSession, GetExtensionSet).WillByDefault(ReturnRef(objMtcExtensionSet));
    ON_CALL(objMessageUtils, GetResponseStatusCode(&objSession, IMessage::SESSION_START, 0))
            .WillByDefault(Return(SipStatusCode::SC_199));
    MockIMessage objMessage;
    ON_CALL(objMessageUtils, GetPreviousResponse(&objSession, IMessage::SESSION_START, 0))
            .WillByDefault(Return(&objMessage));

    EXPECT_EQ(CallStateName::OUTGOING, pOutgoingState->SessionRprReceived(&objSession, 0));
}

TEST_F(OutgoingStateTest, SessionRprReceivedTerminatesCallIfSdpOaFails)
{
    MtcExtensionSet objMtcExtensionSet(GetTestExtensionSet(AString("supportedExtension")));
    ON_CALL(objMtcSession, GetExtensionSet).WillByDefault(ReturnRef(objMtcExtensionSet));
    ON_CALL(objMessageUtils, GetResponseStatusCode(&objSession, IMessage::SESSION_START, 0))
            .WillByDefault(Return(SipStatusCode::SC_180));
    MockIMessage objMessage;
    ON_CALL(objMessageUtils, GetPreviousResponse(&objSession, IMessage::SESSION_START, 0))
            .WillByDefault(Return(&objMessage));
    EXPECT_CALL(objMediaManager, UpdatePemType(&objSession, &objMessage));

    SetSdpOaFailure(objMessage);

    EXPECT_CALL(objMtcSession, SendPrack(IMS_FALSE));
    EXPECT_CALL(objMtcSession, Terminate(_, CallReasonInfo(CODE_MEDIA_NOT_ACCEPTABLE)));
    EXPECT_CALL(objUiNotifier, SendStartFailed(CallReasonInfo(CODE_MEDIA_NOT_ACCEPTABLE)));
    EXPECT_EQ(CallStateName::TERMINATING, pOutgoingState->SessionRprReceived(&objSession, 0));
}

TEST_F(OutgoingStateTest, SessionRprReceivedTerminatesCallIfNoCodecMatched)
{
    MtcExtensionSet objMtcExtensionSet(GetTestExtensionSet(AString("supportedExtension")));
    ON_CALL(objMtcSession, GetExtensionSet).WillByDefault(ReturnRef(objMtcExtensionSet));
    ON_CALL(objMessageUtils, GetResponseStatusCode(&objSession, IMessage::SESSION_START, 0))
            .WillByDefault(Return(SipStatusCode::SC_180));
    MockIMessage objMessage;
    ON_CALL(objMessageUtils, GetPreviousResponse(&objSession, IMessage::SESSION_START, 0))
            .WillByDefault(Return(&objMessage));
    EXPECT_CALL(objMediaManager, UpdatePemType(&objSession, &objMessage));

    const NegotiationResult eNegoResult = NegotiationResult::ERROR_NO_CODEC_MATCHED;
    SetNegotiateSdpFailure(objMessage, eNegoResult);

    EXPECT_CALL(objMtcSession, SendPrack(IMS_FALSE));
    EXPECT_CALL(objMtcSession, Terminate(_, CallReasonInfo(CODE_MEDIA_NOT_ACCEPTABLE)));
    EXPECT_CALL(objUiNotifier, SendStartFailed(CallReasonInfo(CODE_MEDIA_NOT_ACCEPTABLE)));
    EXPECT_EQ(CallStateName::TERMINATING, pOutgoingState->SessionRprReceived(&objSession, 0));
}

TEST_F(OutgoingStateTest, SessionRprReceivedTerminatesCallIfInvalidDescriptor)
{
    MtcExtensionSet objMtcExtensionSet(GetTestExtensionSet(AString("supportedExtension")));
    ON_CALL(objMtcSession, GetExtensionSet).WillByDefault(ReturnRef(objMtcExtensionSet));
    ON_CALL(objMessageUtils, GetResponseStatusCode(&objSession, IMessage::SESSION_START, 0))
            .WillByDefault(Return(SipStatusCode::SC_180));
    MockIMessage objMessage;
    ON_CALL(objMessageUtils, GetPreviousResponse(&objSession, IMessage::SESSION_START, 0))
            .WillByDefault(Return(&objMessage));
    EXPECT_CALL(objMediaManager, UpdatePemType(&objSession, &objMessage));

    const NegotiationResult eNegoResult = NegotiationResult::ERROR_INVALID_DESCRIPTOR;
    SetNegotiateSdpFailure(objMessage, eNegoResult);

    EXPECT_CALL(objMtcSession, SendPrack(IMS_FALSE));
    EXPECT_CALL(objMtcSession, Terminate(_, CallReasonInfo(CODE_REJECT_UNSUPPORTED_SDP_HEADERS)));
    EXPECT_CALL(
            objUiNotifier, SendStartFailed(CallReasonInfo(CODE_REJECT_UNSUPPORTED_SDP_HEADERS)));
    EXPECT_EQ(CallStateName::TERMINATING, pOutgoingState->SessionRprReceived(&objSession, 0));
}

TEST_F(OutgoingStateTest, SessionRprReceivedUpdatesQosPreconditionInfo)
{
    MtcExtensionSet objMtcExtensionSet(GetTestExtensionSet(AString("supportedExtension")));
    ON_CALL(objMtcSession, GetExtensionSet).WillByDefault(ReturnRef(objMtcExtensionSet));
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
    MtcExtensionSet objMtcExtensionSet(GetTestExtensionSet(AString("supportedExtension")));
    ON_CALL(objMtcSession, GetExtensionSet).WillByDefault(ReturnRef(objMtcExtensionSet));
    ON_CALL(objMessageUtils, GetResponseStatusCode(&objSession, IMessage::SESSION_START, 0))
            .WillByDefault(Return(SipStatusCode::SC_180));
    MockIMessage objMessage;
    ON_CALL(objMessageUtils, GetPreviousResponse(&objSession, IMessage::SESSION_START, 0))
            .WillByDefault(Return(&objMessage));

    ON_CALL(objMediaManager, GetRemoteRtpPort(_, MEDIATYPE_AUDIO)).WillByDefault(Return(0));

    EXPECT_CALL(objMtcSession, SendPrack(IMS_FALSE));
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
    MtcExtensionSet objMtcExtensionSet(GetTestExtensionSet(AString("supportedExtension")));
    ON_CALL(objMtcSession, GetExtensionSet).WillByDefault(ReturnRef(objMtcExtensionSet));
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
    MtcExtensionSet objMtcExtensionSet(GetTestExtensionSet(AString("supportedExtension")));
    ON_CALL(objMtcSession, GetExtensionSet).WillByDefault(ReturnRef(objMtcExtensionSet));
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

TEST_F(OutgoingStateTest, SessionRprReceivedInvokesStartWatchdogFor180WithSdpAnswer)
{
    MtcExtensionSet objMtcExtensionSet(GetTestExtensionSet(AString("supportedExtension")));
    ON_CALL(objMtcSession, GetExtensionSet).WillByDefault(ReturnRef(objMtcExtensionSet));
    ON_CALL(objMessageUtils, GetResponseStatusCode(&objSession, IMessage::SESSION_START, 0))
            .WillByDefault(Return(SipStatusCode::SC_180));
    MockIMessage objMessage;
    ON_CALL(objMessageUtils, GetPreviousResponse(&objSession, IMessage::SESSION_START, 0))
            .WillByDefault(Return(&objMessage));

    ON_CALL(objMtcSession, SendPrack(IMS_FALSE)).WillByDefault(Return(IMS_SUCCESS));

    MockISipMessage objSipMessage;
    ON_CALL(objMessage, GetMessage).WillByDefault(Return(&objSipMessage));
    SetSdpOaSuccessWithSdp(objMessage, objSipMessage);  // to cover StartEpsFallbackWatchdogIfNeeded

    ON_CALL(*pConfigurationProxy, GetInt(ConfigVoice::KEY_EPS_FALLBACK_WATCHDOG_TIME_MILLIS_INT))
            .WillByDefault(Return(2000));
    ON_CALL(objService, IsNr).WillByDefault(Return(IMS_TRUE));

    EXPECT_CALL(*pEpsFbTrigger, StartWatchdog);

    EXPECT_EQ(CallStateName::OUTGOING, pOutgoingState->SessionRprReceived(&objSession, 0));
}

TEST_F(OutgoingStateTest, SessionRprReceivedInvokesStartWatchdogFor181WithSdpAnswer)
{
    MtcExtensionSet objMtcExtensionSet(GetTestExtensionSet(AString("supportedExtension")));
    ON_CALL(objMtcSession, GetExtensionSet).WillByDefault(ReturnRef(objMtcExtensionSet));
    ON_CALL(objMessageUtils, GetResponseStatusCode(&objSession, IMessage::SESSION_START, 0))
            .WillByDefault(Return(SipStatusCode::SC_181));
    MockIMessage objMessage;
    ON_CALL(objMessageUtils, GetPreviousResponse(&objSession, IMessage::SESSION_START, 0))
            .WillByDefault(Return(&objMessage));

    ON_CALL(objMtcSession, SendPrack(IMS_FALSE)).WillByDefault(Return(IMS_SUCCESS));

    MockISipMessage objSipMessage;
    ON_CALL(objMessage, GetMessage).WillByDefault(Return(&objSipMessage));
    SetSdpOaSuccessWithSdp(objMessage, objSipMessage);  // to cover StartEpsFallbackWatchdogIfNeeded

    ON_CALL(*pConfigurationProxy, GetInt(ConfigVoice::KEY_EPS_FALLBACK_WATCHDOG_TIME_MILLIS_INT))
            .WillByDefault(Return(2000));
    ON_CALL(objService, IsNr).WillByDefault(Return(IMS_TRUE));

    EXPECT_CALL(*pEpsFbTrigger, StartWatchdog);

    EXPECT_EQ(CallStateName::OUTGOING, pOutgoingState->SessionRprReceived(&objSession, 0));
}

TEST_F(OutgoingStateTest, SessionRprReceivedInvokesStartWatchdogFor182WithSdpAnswer)
{
    MtcExtensionSet objMtcExtensionSet(GetTestExtensionSet(AString("supportedExtension")));
    ON_CALL(objMtcSession, GetExtensionSet).WillByDefault(ReturnRef(objMtcExtensionSet));
    ON_CALL(objMessageUtils, GetResponseStatusCode(&objSession, IMessage::SESSION_START, 0))
            .WillByDefault(Return(SipStatusCode::SC_182));
    MockIMessage objMessage;
    ON_CALL(objMessageUtils, GetPreviousResponse(&objSession, IMessage::SESSION_START, 0))
            .WillByDefault(Return(&objMessage));

    ON_CALL(objMtcSession, SendPrack(IMS_FALSE)).WillByDefault(Return(IMS_SUCCESS));

    MockISipMessage objSipMessage;
    ON_CALL(objMessage, GetMessage).WillByDefault(Return(&objSipMessage));
    SetSdpOaSuccessWithSdp(objMessage, objSipMessage);  // to cover StartEpsFallbackWatchdogIfNeeded

    ON_CALL(*pConfigurationProxy, GetInt(ConfigVoice::KEY_EPS_FALLBACK_WATCHDOG_TIME_MILLIS_INT))
            .WillByDefault(Return(2000));
    ON_CALL(objService, IsNr).WillByDefault(Return(IMS_TRUE));

    EXPECT_CALL(*pEpsFbTrigger, StartWatchdog);

    EXPECT_EQ(CallStateName::OUTGOING, pOutgoingState->SessionRprReceived(&objSession, 0));
}

TEST_F(OutgoingStateTest, SessionRprReceivedInvokesStartWatchdogFor183WithSdpAnswer)
{
    MtcExtensionSet objMtcExtensionSet(GetTestExtensionSet(AString("supportedExtension")));
    ON_CALL(objMtcSession, GetExtensionSet).WillByDefault(ReturnRef(objMtcExtensionSet));
    ON_CALL(objMessageUtils, GetResponseStatusCode(&objSession, IMessage::SESSION_START, 0))
            .WillByDefault(Return(SipStatusCode::SC_183));
    MockIMessage objMessage;
    ON_CALL(objMessageUtils, GetPreviousResponse(&objSession, IMessage::SESSION_START, 0))
            .WillByDefault(Return(&objMessage));

    ON_CALL(objMtcSession, SendPrack(IMS_FALSE)).WillByDefault(Return(IMS_SUCCESS));

    MockISipMessage objSipMessage;
    ON_CALL(objMessage, GetMessage).WillByDefault(Return(&objSipMessage));
    SetSdpOaSuccessWithSdp(objMessage, objSipMessage);  // to cover StartEpsFallbackWatchdogIfNeeded

    ON_CALL(*pConfigurationProxy, GetInt(ConfigVoice::KEY_EPS_FALLBACK_WATCHDOG_TIME_MILLIS_INT))
            .WillByDefault(Return(2000));
    ON_CALL(objService, IsNr).WillByDefault(Return(IMS_TRUE));

    EXPECT_CALL(*pEpsFbTrigger, StartWatchdog);

    EXPECT_EQ(CallStateName::OUTGOING, pOutgoingState->SessionRprReceived(&objSession, 0));
}

TEST_F(OutgoingStateTest,
        SessionRprReceivedDoesNotSendLocalResourceConfirmationWithPrackIfNotRequired)
{
    MtcExtensionSet objMtcExtensionSet(GetTestExtensionSet(AString("supportedExtension")));
    ON_CALL(objMtcSession, GetExtensionSet).WillByDefault(ReturnRef(objMtcExtensionSet));
    ON_CALL(objMessageUtils, GetResponseStatusCode(&objSession, IMessage::SESSION_START, 0))
            .WillByDefault(Return(SipStatusCode::SC_183));
    MockIMessage objMessage;
    ON_CALL(objMessageUtils, GetPreviousResponse(&objSession, IMessage::SESSION_START, 0))
            .WillByDefault(Return(&objMessage));

    ON_CALL(objPreconditionManager, IsLocalResourceConfirmationRequired(&objSession))
            .WillByDefault(Return(IMS_FALSE));

    EXPECT_CALL(objMediaManager, AdjustDirectionForLocalResourceConfirmation(_)).Times(0);
    EXPECT_CALL(objMtcSession, SendPrack(IMS_FALSE)).WillOnce(Return(IMS_SUCCESS));
    EXPECT_CALL(objPreconditionManager, OnMessageReceived(&objSession, &objMessage));
    EXPECT_CALL(objMediaManager, Run(&objSession, &objMessage, IMS_TRUE));
    EXPECT_CALL(objUiNotifier, SendProgressing());

    EXPECT_EQ(CallStateName::OUTGOING, pOutgoingState->SessionRprReceived(&objSession, 0));
}

TEST_F(OutgoingStateTest,
        SessionRprReceivedDoesNotSendLocalResourceConfirmationWithPrackIfResourceNotReserved)
{
    MtcExtensionSet objMtcExtensionSet(GetTestExtensionSet(AString("supportedExtension")));
    ON_CALL(objMtcSession, GetExtensionSet).WillByDefault(ReturnRef(objMtcExtensionSet));
    ON_CALL(objMessageUtils, GetResponseStatusCode(&objSession, IMessage::SESSION_START, 0))
            .WillByDefault(Return(SipStatusCode::SC_183));
    MockIMessage objMessage;
    ON_CALL(objMessageUtils, GetPreviousResponse(&objSession, IMessage::SESSION_START, 0))
            .WillByDefault(Return(&objMessage));

    ON_CALL(objPreconditionManager, IsLocalResourceConfirmationRequired(&objSession))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objPreconditionManager, IsAvailableToSendLocalResourceConfirmation(&objSession))
            .WillByDefault(Return(IMS_FALSE));

    EXPECT_CALL(objMediaManager, AdjustDirectionForLocalResourceConfirmation(_)).Times(0);
    EXPECT_CALL(objMtcSession, SendPrack(IMS_FALSE)).WillOnce(Return(IMS_SUCCESS));
    EXPECT_CALL(objPreconditionManager, OnMessageReceived(&objSession, &objMessage));
    EXPECT_CALL(objMediaManager, Run(&objSession, &objMessage, IMS_TRUE));
    EXPECT_CALL(objUiNotifier, SendProgressing());

    EXPECT_EQ(CallStateName::OUTGOING, pOutgoingState->SessionRprReceived(&objSession, 0));
}

TEST_F(OutgoingStateTest,
        SessionRprReceivedInvokesLocalResourceConfirmationWithPrackIfResourceReserved)
{
    MtcExtensionSet objMtcExtensionSet(GetTestExtensionSet(AString("supportedExtension")));
    ON_CALL(objMtcSession, GetExtensionSet).WillByDefault(ReturnRef(objMtcExtensionSet));
    ON_CALL(objMessageUtils, GetResponseStatusCode(&objSession, IMessage::SESSION_START, 0))
            .WillByDefault(Return(SipStatusCode::SC_183));
    MockIMessage objMessage;
    ON_CALL(objMessageUtils, GetPreviousResponse(&objSession, IMessage::SESSION_START, 0))
            .WillByDefault(Return(&objMessage));

    ON_CALL(objPreconditionManager, IsLocalResourceConfirmationRequired(&objSession))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objPreconditionManager, IsAvailableToSendLocalResourceConfirmation(&objSession))
            .WillByDefault(Return(IMS_TRUE));

    EXPECT_CALL(objMediaManager, AdjustDirectionForLocalResourceConfirmation(_)).Times(1);
    EXPECT_CALL(objMtcSession, SendPrack(IMS_TRUE)).WillOnce(Return(IMS_SUCCESS));
    EXPECT_CALL(objPreconditionManager, OnMessageReceived(&objSession, &objMessage));
    EXPECT_CALL(objMediaManager, Run(&objSession, &objMessage, IMS_TRUE));
    EXPECT_CALL(objUiNotifier, SendProgressing());

    EXPECT_EQ(CallStateName::OUTGOING, pOutgoingState->SessionRprReceived(&objSession, 0));
}

TEST_F(OutgoingStateTest, SessionRprReceivedInvokesSendPrackIfFinalResponseIsNotReceived)
{
    MtcExtensionSet objMtcExtensionSet(GetTestExtensionSet(AString("supportedExtension")));
    ON_CALL(objMtcSession, GetExtensionSet).WillByDefault(ReturnRef(objMtcExtensionSet));
    ON_CALL(objMessageUtils, GetResponseStatusCode(&objSession, IMessage::SESSION_START, 0))
            .WillByDefault(Return(SipStatusCode::SC_180));
    MockIMessage objMessage;
    ON_CALL(objMessageUtils, GetPreviousResponse(&objSession, IMessage::SESSION_START, 0))
            .WillByDefault(Return(&objMessage));

    ON_CALL(objSession, IsFinalResponseReceivedForInitialInviteRequest)
            .WillByDefault(Return(IMS_FALSE));

    EXPECT_CALL(objMtcSession, SendPrack(_));

    EXPECT_EQ(CallStateName::OUTGOING, pOutgoingState->SessionRprReceived(&objSession, 0));
}

TEST_F(OutgoingStateTest, SessionRprReceivedDoesNotInvokeSendPrackIfFinalResponseIsReceived)
{
    MtcExtensionSet objMtcExtensionSet(GetTestExtensionSet(AString("supportedExtension")));
    ON_CALL(objMtcSession, GetExtensionSet).WillByDefault(ReturnRef(objMtcExtensionSet));
    ON_CALL(objMessageUtils, GetResponseStatusCode(&objSession, IMessage::SESSION_START, 0))
            .WillByDefault(Return(SipStatusCode::SC_180));
    MockIMessage objMessage;
    ON_CALL(objMessageUtils, GetPreviousResponse(&objSession, IMessage::SESSION_START, 0))
            .WillByDefault(Return(&objMessage));

    ON_CALL(objSession, IsFinalResponseReceivedForInitialInviteRequest)
            .WillByDefault(Return(IMS_TRUE));

    EXPECT_CALL(objMtcSession, SendPrack(_)).Times(0);

    EXPECT_EQ(CallStateName::OUTGOING, pOutgoingState->SessionRprReceived(&objSession, 0));
}

TEST_F(OutgoingStateTest, SessionRprReceivedWithNegoFailureTerminatesSessionAndKeepsCurrentState)
{
    MockIMtcSession objMtcSession2;
    objSessions.Append(&objMtcSession2);
    ON_CALL(objCallContext, GetSessions()).WillByDefault(ReturnRef(objSessions));

    MtcExtensionSet objMtcExtensionSet(GetTestExtensionSet(AString("supportedExtension")));
    ON_CALL(objMtcSession, GetExtensionSet).WillByDefault(ReturnRef(objMtcExtensionSet));
    ON_CALL(objMessageUtils, GetResponseStatusCode(&objSession, IMessage::SESSION_START, 0))
            .WillByDefault(Return(SipStatusCode::SC_183));
    MockIMessage objMessage;
    ON_CALL(objMessageUtils, GetPreviousResponse(&objSession, IMessage::SESSION_START, 0))
            .WillByDefault(Return(&objMessage));
    SetSdpOaFailure(objMessage);

    EXPECT_CALL(objMtcSession, Terminate(IMS_TRUE, CallReasonInfo(CODE_SIP_NOT_ACCEPTABLE)));
    EXPECT_CALL(objUiNotifier, SendStartFailed(_)).Times(0);
    EXPECT_CALL(objCallContext, RemoveSession(Ref(objMtcSession))).Times(1);
    EXPECT_CALL(objCallContext, RemoveSession(Ref(objMtcSession2))).Times(0);

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
