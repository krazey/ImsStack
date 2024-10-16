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
#include "ISipClientConnection.h"
#include "ImsTypeDef.h"
#include "MockIMessage.h"
#include "MockIMtcService.h"
#include "MockISession.h"
#include "MockISipClientConnection.h"
#include "MockISipMessage.h"
#include "MockISipServerConnection.h"
#include "MtcDef.h"
#include "SipMethod.h"
#include "SipStatusCode.h"
#include "aos/ImsAosReason.h"
#include "call/IMtcCall.h"
#include "call/MockEpsFallbackTrigger.h"
#include "call/MockIMtcCall.h"
#include "call/MockIMtcCallContext.h"
#include "call/MockIMtcCallManager.h"
#include "call/MockIMtcSession.h"
#include "call/MockIMtcUiNotifier.h"
#include "call/TestMtcPendingOperationHolder.h"
#include "call/UpdatingInfo.h"
#include "call/block/IMtcBlockChecker.h"
#include "call/block/MockIMtcBlockChecker.h"
#include "call/extension/MockIMtcExtension.h"
#include "call/extension/MtcExtensionSet.h"
#include "call/state/EstablishedState.h"
#include "call/state/MtcCallState.h"
#include "conferencecall/MockIConferenceController.h"
#include "conferencecall/MockIConferenceManager.h"
#include "configuration/MockIMtcConfigurationManager.h"
#include "configuration/MtcConfigurationProxy.h"
#include "helper/ISrvccStateListener.h"
#include "helper/MockMtcTimerWrapper.h"
#include "helper/MtcSupplementaryService.h"
#include "media/MockIMtcMediaManager.h"
#include "precondition/MockIMtcPreconditionManager.h"
#include "ussi/MockUssiController.h"
#include "utility/MockIMessageUtils.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;

namespace android
{

// SipMethod
MATCHER_P(IsEqualSipMethod, method, "")
{
    return arg.Equals(method);
}

MATCHER_P(IsSameCallReasonInfoCode, code, "")
{
    return arg.nCode == code;
}

MATCHER(IsEmptyString, "")
{
    return arg.IsEmpty();
}

class EstablishedStateTest : public ::testing::Test
{
public:
    EstablishedState* pEstablishedState;
    MockIMtcCallContext objMockCallContext;
    MockIMtcService objService;
    MockIMtcMediaManager objMockMediaManager;
    MockIMtcSession objMockMtcSession;
    MockIMtcUiNotifier objUiNotifier;
    MockISession objMockISession;
    MockIMtcConfigurationManager* pMockConfigurationManager;
    MockIMtcPreconditionManager objMockPreconditionManager;
    MockMtcTimerWrapper objTimerWrapper;
    MockIMessage objMessage;
    MockIMessageUtils objMessageUtils;
    MockIMtcBlockChecker* pBlockChecker;
    MtcConfigurationProxy* pConfigurationProxy;
    UpdatingInfo* pUpdatingInfo;
    MediaInfo objMediaInfo;
    CallInfo objCallInfo;
    MtcSupplementaryService* pSupplementaryService;
    MockIMtcCallManager objMockIMtcCallManager;
    MockISipClientConnection objMockISipClientConnection;
    MockUssiController* pMockUssiController;

protected:
    virtual void SetUp() override
    {
        ON_CALL(objMockCallContext, GetService).WillByDefault(ReturnRef(objService));
        ON_CALL(objMockCallContext, GetMediaManager).WillByDefault(ReturnRef(objMockMediaManager));
        ON_CALL(objMockMediaManager, GetMediaInfo).WillByDefault(ReturnRef(objMediaInfo));
        ON_CALL(objMockCallContext, GetCallInfo()).WillByDefault(ReturnRef(objCallInfo));
        ON_CALL(objMockCallContext, GetSession()).WillByDefault(Return(&objMockMtcSession));
        ON_CALL(objMockMtcSession, GetISession).WillByDefault(ReturnRef(objMockISession));
        ON_CALL(objMockISession, GetPreviousRequest(_)).WillByDefault(Return(&objMessage));

        ON_CALL(objMockCallContext, GetUiNotifier).WillByDefault(ReturnRef(objUiNotifier));

        pMockConfigurationManager = new MockIMtcConfigurationManager();
        pConfigurationProxy = new MtcConfigurationProxy(pMockConfigurationManager);
        ON_CALL(objMockCallContext, GetConfigurationProxy)
                .WillByDefault(ReturnRef(*pConfigurationProxy));

        pUpdatingInfo = new UpdatingInfo(objMockCallContext);
        ON_CALL(objMockCallContext, GetUpdatingInfo).WillByDefault(ReturnRef(*pUpdatingInfo));
        ON_CALL(objMockCallContext, GetPreconditionManager)
                .WillByDefault(ReturnRef(objMockPreconditionManager));
        ON_CALL(objMockMediaManager, FormSdp(&objMockISession, CallType::VOIP, IMS_TRUE))
                .WillByDefault(Return(IMS_SUCCESS));

        ON_CALL(objMockCallContext, GetMessageUtils).WillByDefault(ReturnRef(objMessageUtils));

        ON_CALL(objMockPreconditionManager, FormPreconditionSdp(_, _)).WillByDefault(Return());

        ON_CALL(objMockCallContext, GetTimer).WillByDefault(ReturnRef(objTimerWrapper));
        pSupplementaryService =
                new MtcSupplementaryService(objMockCallContext, *pConfigurationProxy);
        ON_CALL(objMockCallContext, GetSupplementaryService)
                .WillByDefault(ReturnRef(*pSupplementaryService));

        // This will be deleted by EstablishedState.
        pBlockChecker = new MockIMtcBlockChecker();
        ON_CALL(objMockCallContext, CreateBlockChecker).WillByDefault(Return(pBlockChecker));
        ON_CALL(objMockCallContext, GetCallManager)
                .WillByDefault(ReturnRef(objMockIMtcCallManager));
        ON_CALL(objMockCallContext, CreateClientConnection(IsEqualSipMethod(SipMethod::INFO)))
                .WillByDefault(Return(&objMockISipClientConnection));
        pMockUssiController = new MockUssiController(objMockCallContext, new UssiDataParser());
        ON_CALL(objMockCallContext, GetUssiController).WillByDefault(Return(pMockUssiController));

        pEstablishedState = new EstablishedState(objMockCallContext);
    }

    virtual void TearDown() override
    {
        delete pConfigurationProxy;
        delete pUpdatingInfo;
        delete pEstablishedState;
        delete pSupplementaryService;
        delete pMockUssiController;
    }

    MtcExtensionSet GetTestExtensionSet(IN const AString& strOptionTag, IN const IMS_BOOL& bSupoort)
    {
        ImsList<IMtcExtension*> objExtensions;
        MockIMtcExtension* pExtension = new MockIMtcExtension();
        ON_CALL(*pExtension, GetOptionTag).WillByDefault(ReturnRef(strOptionTag));
        ON_CALL(*pExtension, IsAvailableOnRemote).WillByDefault(Return(bSupoort));
        objExtensions.Append(pExtension);
        MtcExtensionSet objMtcExtensionSet(objMockCallContext, objExtensions);
        return objMtcExtensionSet;
    }
};

TEST_F(EstablishedStateTest, OnEnterRunsPendingOperationSynchronously)
{
    ON_CALL(objMockISession, IsSessionRefreshInProgress).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objTimerWrapper, IsActive(MtcCallState::TIMER_DELAY_UPDATE_AFTER_CONNECTED))
            .WillByDefault(Return(IMS_FALSE));

    EXPECT_CALL(objMockCallContext, RunPendingOperationIfPossible());
    pEstablishedState->OnEnter();
}

TEST_F(EstablishedStateTest,
        OnEnterRunsPendingOperationAsynchronouslyIfOnRefreshingOrTimerForDelayingUpdateIsActive)
{
    EXPECT_CALL(objMockISession, IsSessionRefreshInProgress)
            .Times(2)
            .WillOnce(Return(IMS_TRUE))
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(objTimerWrapper, IsActive(MtcCallState::TIMER_DELAY_UPDATE_AFTER_CONNECTED))
            .Times(1)
            .WillOnce(Return(IMS_TRUE));

    EXPECT_CALL(objMockCallContext, GetAsyncRunner(_)).Times(2);
    pEstablishedState->OnEnter();
    pEstablishedState->OnEnter();
}

TEST_F(EstablishedStateTest, TerminateByUserActionWhenNoReceivingAudioPackets)
{
    EXPECT_CALL(objMockMediaManager, IsAudioInactive)
            .Times(2)
            .WillOnce(Return(IMS_TRUE))
            .WillOnce(Return(IMS_FALSE));

    EXPECT_CALL(objMockMtcSession, Terminate(_, _)).Times(2);

    CallReasonInfo objReason(CODE_USER_TERMINATED);
    pEstablishedState->Terminate(objReason);
    pEstablishedState->Terminate(objReason);
}

TEST_F(EstablishedStateTest, SessionTerminatedInvokesSendTerminated)
{
    ImsList<IMtcCall*> objEmptyCalls;
    ON_CALL(objMockIMtcCallManager, GetCallsInConference).WillByDefault(Return(objEmptyCalls));
    ON_CALL(objMockISession, GetTerminationReason)
            .WillByDefault(Return(ISession::TERMINATION_REASON_USER_ACTION));
    EXPECT_CALL(objUiNotifier, SendTerminated(IsSameCallReasonInfoCode(CODE_USER_TERMINATED)));

    pEstablishedState->SessionTerminated(&objMockISession);
}

TEST_F(EstablishedStateTest, SessionTerminatedInvokesSendTerminatedWithConf)
{
    MockIConferenceManager objMockConferenceManager;
    MockIConferenceController objMockConferenceController;
    ON_CALL(objMockCallContext, GetConferenceManager())
            .WillByDefault(ReturnRef(objMockConferenceManager));
    ON_CALL(objMockConferenceManager, GetController(_))
            .WillByDefault(Return(&objMockConferenceController));
    ON_CALL(objMockConferenceController, GetCallStatusInConference(_))
            .WillByDefault(Return(IndividualCallState::JOINING));

    ImsList<IMtcCall*> objConfCalls;
    MockIMtcCall objMockMtcCall;
    objConfCalls.Append(&objMockMtcCall);
    ON_CALL(objMockIMtcCallManager, GetCallsInConference).WillByDefault(Return(objConfCalls));
    ON_CALL(objMockISession, GetTerminationReason)
            .WillByDefault(Return(ISession::TERMINATION_REASON_USER_ACTION));
    EXPECT_CALL(objUiNotifier,
            SendTerminated(IsSameCallReasonInfoCode(CODE_LOCAL_ENDED_BY_CONFERENCE_MERGE)));

    pEstablishedState->SessionTerminated(&objMockISession);
}

TEST_F(EstablishedStateTest, RefreshNotifyCompletedRunsPendingOperationAsynchronously)
{
    EXPECT_CALL(objMockCallContext, GetAsyncRunner(_));

    ISipClientConnection* piFakeConnection = reinterpret_cast<ISipClientConnection*>(0x1);
    pEstablishedState->Refresh_NotifyCompleted(piFakeConnection);
}

TEST_F(EstablishedStateTest, OnReceivingMediaDataFailedWithVideoInvokesDowngrade)
{
    EXPECT_CALL(objTimerWrapper, IsActive(MtcCallState::TIMER_DELAY_UPDATE_AFTER_CONNECTED))
            .WillRepeatedly(Return(IMS_FALSE));
    EXPECT_CALL(objMockMtcSession, Update(UpdateType::SESSION, IMS_FALSE, SipMethod::INVITE))
            .Times(2)
            .WillOnce(Return(IMS_SUCCESS))
            .WillOnce(Return(IMS_FAILURE));
    EXPECT_CALL(objTimerWrapper, IsActive(MtcCallState::TIMER_CONVERT_REMOTE_RESPONSE))
            .Times(2)
            .WillRepeatedly(Return(IMS_FALSE));
    EXPECT_CALL(objTimerWrapper, Start(MtcCallState::TIMER_CONVERT_REMOTE_RESPONSE, _)).Times(2);

    ON_CALL(objMockMtcSession, GetCallType).WillByDefault(Return(CallType::VT));
    pEstablishedState->OnReceivingMediaDataFailed(MEDIATYPE_VIDEO, MEDIA_PROTOCOL_RTP);

    ON_CALL(objMockMtcSession, GetCallType).WillByDefault(Return(CallType::RTT));
    pEstablishedState->OnReceivingMediaDataFailed(MEDIATYPE_TEXT, MEDIA_PROTOCOL_RTP);
}

TEST_F(EstablishedStateTest, OnVideoLowestBitRateInvokesDowngradeIfCallTypeIsVt)
{
    ON_CALL(objMockMtcSession, GetCallType).WillByDefault(Return(CallType::VT));

    EXPECT_CALL(objTimerWrapper, IsActive(MtcCallState::TIMER_DELAY_UPDATE_AFTER_CONNECTED))
            .WillRepeatedly(Return(IMS_FALSE));
    EXPECT_CALL(objMockMtcSession, SetCallType(CallType::VOIP));
    EXPECT_CALL(objMockMtcSession, Update(UpdateType::SESSION, IMS_FALSE, SipMethod::INVITE))
            .WillOnce(Return(IMS_SUCCESS));
    EXPECT_CALL(objTimerWrapper, IsActive(MtcCallState::TIMER_CONVERT_REMOTE_RESPONSE))
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(objTimerWrapper, Start(MtcCallState::TIMER_CONVERT_REMOTE_RESPONSE, _)).Times(1);

    pEstablishedState->OnVideoLowestBitRate();
}

TEST_F(EstablishedStateTest, OnVideoLowestBitRateInvokesDowngradeIfCallTypeIsVideoRtt)
{
    ON_CALL(objMockMtcSession, GetCallType).WillByDefault(Return(CallType::VIDEO_RTT));

    EXPECT_CALL(objTimerWrapper, IsActive(MtcCallState::TIMER_DELAY_UPDATE_AFTER_CONNECTED))
            .WillRepeatedly(Return(IMS_FALSE));
    EXPECT_CALL(objMockMtcSession, SetCallType(CallType::RTT));
    EXPECT_CALL(objMockMtcSession, Update(UpdateType::SESSION, IMS_FALSE, SipMethod::INVITE))
            .WillOnce(Return(IMS_SUCCESS));
    EXPECT_CALL(objTimerWrapper, IsActive(MtcCallState::TIMER_CONVERT_REMOTE_RESPONSE))
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(objTimerWrapper, Start(MtcCallState::TIMER_CONVERT_REMOTE_RESPONSE, _)).Times(1);

    pEstablishedState->OnVideoLowestBitRate();
}

TEST_F(EstablishedStateTest, OnMediaFailed)
{
    EXPECT_CALL(objMockMtcSession, Terminate(IMS_TRUE, CallReasonInfo(CODE_MEDIA_INIT_FAILED)))
            .Times(1);

    pEstablishedState->OnMediaFailed(CallReasonInfo(CODE_MEDIA_INIT_FAILED));
}

TEST_F(EstablishedStateTest, QosReserveFailedInvokesSendTerminated)
{
    EXPECT_CALL(objMockMtcSession,
            Terminate(IMS_TRUE, CallReasonInfo(CODE_LOCAL_CALL_RESOURCE_RESERVATION_FAILED)))
            .Times(1);
    EXPECT_CALL(objUiNotifier,
            SendTerminated(CallReasonInfo(CODE_LOCAL_CALL_RESOURCE_RESERVATION_FAILED)));

    EXPECT_EQ(CallStateName::TERMINATING,
            pEstablishedState->QosReserveFailed(&objMockISession, QosLossPolicy::RELEASE));
}

TEST_F(EstablishedStateTest, QosReserveFailedDowngradesVtCallToVoipCall)
{
    ON_CALL(objMockMtcSession, GetCallType).WillByDefault(Return(CallType::VT));
    EXPECT_CALL(objMockMtcSession, SetCallType(CallType::VOIP));
    EXPECT_CALL(objMockMtcSession, Update(UpdateType::SESSION, IMS_FALSE, SipMethod::INVITE));

    EXPECT_EQ(CallStateName::UPDATING,
            pEstablishedState->QosReserveFailed(&objMockISession, QosLossPolicy::MODIFY));
}

TEST_F(EstablishedStateTest, OnIpcanChangedNotHandledIfConfigurationIsOff)
{
    ON_CALL(*pMockConfigurationManager, IsEnableSendReinviteOnRatChange)
            .WillByDefault(Return(IMS_FALSE));

    TestMtcPendingOperationHolder objPendingOperationHolder;
    ON_CALL(objMockCallContext, GetPendingOperationHolder)
            .WillByDefault(ReturnRef(objPendingOperationHolder));

    EXPECT_CALL(objPendingOperationHolder.GetMock(), OnIpcanChanged(_)).Times(0);

    IMS_UINT32 eIpcan = 1;
    pEstablishedState->OnIpcanChanged(eIpcan);
}

TEST_F(EstablishedStateTest,
        OnIpcanChangedDoesNotPushPendingOperationIfNoSessionRefreshingAndTimerForDelayingUpdateIsNotActive)
{
    ON_CALL(*pMockConfigurationManager, IsEnableSendReinviteOnRatChange)
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objMockISession, IsSessionRefreshInProgress).WillByDefault(Return(IMS_FALSE));
    EXPECT_CALL(objTimerWrapper, IsActive(MtcCallState::TIMER_DELAY_UPDATE_AFTER_CONNECTED))
            .WillRepeatedly(Return(IMS_FALSE));

    TestMtcPendingOperationHolder objPendingOperationHolder;
    ON_CALL(objMockCallContext, GetPendingOperationHolder)
            .WillByDefault(ReturnRef(objPendingOperationHolder));

    EXPECT_CALL(objPendingOperationHolder.GetMock(), OnIpcanChanged(_)).Times(0);
    EXPECT_CALL(objMockMtcSession, Update(_, _, _));
    EXPECT_CALL(objTimerWrapper, IsActive(MtcCallState::TIMER_CONVERT_REMOTE_RESPONSE))
            .WillRepeatedly(Return(IMS_FALSE));
    EXPECT_CALL(objTimerWrapper, Start(MtcCallState::TIMER_CONVERT_REMOTE_RESPONSE, _));

    IMS_UINT32 eIpcan = 1;
    pEstablishedState->OnIpcanChanged(eIpcan);
}

TEST_F(EstablishedStateTest, OnIpcanChangedPushesPendingOperation)
{
    ON_CALL(*pMockConfigurationManager, IsEnableSendReinviteOnRatChange)
            .WillByDefault(Return(IMS_TRUE));
    EXPECT_CALL(objMockISession, IsSessionRefreshInProgress)
            .Times(2)
            .WillOnce(Return(IMS_TRUE))
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(objTimerWrapper, IsActive(MtcCallState::TIMER_DELAY_UPDATE_AFTER_CONNECTED))
            .Times(1)
            .WillOnce(Return(IMS_TRUE));

    TestMtcPendingOperationHolder objPendingOperationHolder;
    ON_CALL(objMockCallContext, GetPendingOperationHolder)
            .WillByDefault(ReturnRef(objPendingOperationHolder));

    const IMS_UINT32 eIpcan = 1;
    EXPECT_CALL(objPendingOperationHolder.GetMock(), OnIpcanChanged(eIpcan)).Times(2);

    pEstablishedState->OnIpcanChanged(eIpcan);
    pEstablishedState->OnIpcanChanged(eIpcan);
}

TEST_F(EstablishedStateTest, OnTimerExpiredBringsAsyncRunner)
{
    EXPECT_CALL(objMockCallContext, GetAsyncRunner(_));

    pEstablishedState->OnTimerExpired(MtcCallState::TIMER_DELAY_UPDATE_AFTER_CONNECTED);
}

TEST_F(EstablishedStateTest, SendUpdateBySrvccByCanceled)
{
    EXPECT_CALL(objMockMtcSession,
            Update(UpdateType::SRVCC_RECOVERED_CANCEL, IMS_FALSE, SipMethod::INVITE))
            .Times(1);

    EXPECT_EQ(
            CallStateName::UPDATING, pEstablishedState->OnSrvccStateUpdated(SrvccState::CANCELED));
}

TEST_F(EstablishedStateTest, SendUpdateBySrvccByFailed)
{
    EXPECT_CALL(objMockMtcSession,
            Update(UpdateType::SRVCC_RECOVERED_FAILURE, IMS_FALSE, SipMethod::INVITE))
            .Times(1);

    EXPECT_EQ(CallStateName::UPDATING, pEstablishedState->OnSrvccStateUpdated(SrvccState::FAILED));
}

TEST_F(EstablishedStateTest, SendOfferWithFullCapaOnResponseToReInvite)
{
    ON_CALL(objMessageUtils, GetCallType(_, _, _)).WillByDefault(Return(CallType::UNKNOWN));
    ON_CALL(objMessageUtils, HasSdp(&objMessage)).WillByDefault(Return(IMS_FALSE));

    SipMethod objSipMethod(SipMethod::INVITE);
    ON_CALL(objMessage, GetMethod()).WillByDefault(ReturnRef(objSipMethod));
    ON_CALL(objMockCallContext, IsHeldByMe).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMockMtcSession, GetCallType()).WillByDefault(Return(CallType::VOIP));

    EXPECT_CALL(objMockMtcSession, AcceptUpdate()).Times(1).WillOnce(Return(IMS_SUCCESS));

    EXPECT_EQ(CallStateName::UPDATING, pEstablishedState->SessionUpdateReceived(&objMockISession));
}

TEST_F(EstablishedStateTest, SendIncomingUpdateIsInvokedIfUpdateNeedsToAlert)
{
    ON_CALL(*pMockConfigurationManager, GetPolicyForCheckingQosWhileCallUpgrading)
            .WillByDefault(Return(
                    CarrierConfig::ImsVoice::QOS_CHECK_POLICY_ON_UPGRADING_CALL_AFTER_UPGRADE));

    ON_CALL(objMessageUtils, GetCallType(_, _, _)).WillByDefault(Return(CallType::UNKNOWN));
    ON_CALL(objMessageUtils, HasSdp(&objMessage)).WillByDefault(Return(IMS_TRUE));
    ON_CALL(*pBlockChecker, Check)
            .WillByDefault(
                    Return(IMtcBlockChecker::Result(IMtcBlockChecker::Result::Status::UNBLOCKED)));

    // SetUp IsNeedToAlert() true.
    ON_CALL(objMockMtcSession, GetPreviousCallType()).WillByDefault(Return(CallType::VOIP));
    ON_CALL(objMockMediaManager, GetNegotiatedCallType(_)).WillByDefault(Return(CallType::VT));

    EXPECT_CALL(objUiNotifier, SendIncomingUpdate(_));

    EXPECT_EQ(CallStateName::UPDATING, pEstablishedState->SessionUpdateReceived(&objMockISession));
}

TEST_F(EstablishedStateTest, SendProvisionalResponseIsInvokedIfPreconditionIsSupported)
{
    ON_CALL(*pMockConfigurationManager, GetPolicyForCheckingQosWhileCallUpgrading)
            .WillByDefault(Return(
                    CarrierConfig::ImsVoice::QOS_CHECK_POLICY_ON_UPGRADING_CALL_DURING_UPGRADING));

    ON_CALL(objMessageUtils, GetCallType(_, _, _)).WillByDefault(Return(CallType::UNKNOWN));
    ON_CALL(objMessageUtils, HasSdp(&objMessage)).WillByDefault(Return(IMS_TRUE));
    ON_CALL(*pBlockChecker, Check)
            .WillByDefault(
                    Return(IMtcBlockChecker::Result(IMtcBlockChecker::Result::Status::UNBLOCKED)));

    // SetUp IsNeedToAlert() true.
    ON_CALL(objMockMtcSession, GetPreviousCallType()).WillByDefault(Return(CallType::VOIP));
    ON_CALL(objMockMediaManager, GetNegotiatedCallType(_)).WillByDefault(Return(CallType::VT));

    MtcExtensionSet objMtcExtensionSet(
            GetTestExtensionSet(MtcExtensionSet::OPTION_TAG_RPR, IMS_FALSE));
    ON_CALL(objMockMtcSession, GetExtensionSet).WillByDefault(ReturnRef(objMtcExtensionSet));

    EXPECT_CALL(objUiNotifier, SendIncomingUpdate(_)).Times(0);
    EXPECT_CALL(objMockMtcSession, SendProvisionalResponse(IMS_FALSE, IMS_FALSE));

    EXPECT_EQ(CallStateName::UPDATING, pEstablishedState->SessionUpdateReceived(&objMockISession));
}

TEST_F(EstablishedStateTest, SessionUpdateReceivedRejectsIfBlocked)
{
    ON_CALL(objMessageUtils, GetCallType(_, _, _)).WillByDefault(Return(CallType::UNKNOWN));
    ON_CALL(objMessageUtils, HasSdp(&objMessage)).WillByDefault(Return(IMS_TRUE));
    CallReasonInfo objAnyReason(CODE_UNSPECIFIED);
    ON_CALL(*pBlockChecker, Check)
            .WillByDefault(Return(IMtcBlockChecker::Result(
                    IMtcBlockChecker::Result::Status::BLOCKED, objAnyReason)));

    CallType eAnyCallType = CallType::VIDEO_RTT;
    EXPECT_CALL(objMockMtcSession, GetPreviousCallType()).WillOnce(Return(eAnyCallType));
    EXPECT_CALL(objMockMtcSession, SetCallType(eAnyCallType));
    EXPECT_CALL(objMockMtcSession, Reject(objAnyReason));
    EXPECT_CALL(objMockMediaManager, FinalizeSdp(&objMockISession));

    EXPECT_EQ(
            CallStateName::ESTABLISHED, pEstablishedState->SessionUpdateReceived(&objMockISession));
}

TEST_F(EstablishedStateTest, SessionUpdateReceivedInvokesSendIncomingResume)
{
    ON_CALL(objMessageUtils, HasSdp(&objMessage)).WillByDefault(Return(IMS_TRUE));
    ON_CALL(*pBlockChecker, Check)
            .WillByDefault(
                    Return(IMtcBlockChecker::Result(IMtcBlockChecker::Result::Status::UNBLOCKED)));
    ON_CALL(objMockMediaManager, GetNegotiatedCallType(_)).WillByDefault(Return(CallType::VOIP));
    ON_CALL(objMockMtcSession, GetPreviousCallType).WillByDefault(Return(CallType::VOIP));
    objMediaInfo.eAudioDirection = DIRECTION_RECEIVE;
    pUpdatingInfo->GetModifiedInfo().eAudioDirection = DIRECTION_SEND_RECEIVE;
    ON_CALL(*pMockConfigurationManager, IsCheckUiConditionForIncomingResume)
            .WillByDefault(Return(IMS_TRUE));

    EXPECT_CALL(objUiNotifier, SendIncomingResume).Times(1);
    EXPECT_CALL(objTimerWrapper, Start(_, _)).Times(1);

    EXPECT_EQ(CallStateName::UPDATING, pEstablishedState->SessionUpdateReceived(&objMockISession));
}

TEST_F(EstablishedStateTest,
        SessionUpdateReceivedAnswersDirectlyWithoutInteractionWithAnotherModule)
{
    // Hold, Resume without interaction with Java side, Refresh
    ON_CALL(objMessageUtils, HasSdp(&objMessage)).WillByDefault(Return(IMS_TRUE));
    ON_CALL(*pBlockChecker, Check)
            .WillByDefault(
                    Return(IMtcBlockChecker::Result(IMtcBlockChecker::Result::Status::UNBLOCKED)));
    ON_CALL(objMockMediaManager, GetNegotiatedCallType(_)).WillByDefault(Return(CallType::VOIP));
    ON_CALL(objMockMtcSession, GetPreviousCallType).WillByDefault(Return(CallType::VOIP));
    ON_CALL(*pMockConfigurationManager, IsCheckUiConditionForIncomingResume)
            .WillByDefault(Return(IMS_FALSE));
    SipMethod objSipMethod(SipMethod::UPDATE);
    ON_CALL(objMessage, GetMethod()).WillByDefault(ReturnRef(objSipMethod));

    EXPECT_CALL(objMockMtcSession, AcceptUpdate()).Times(1);
    EXPECT_EQ(
            CallStateName::ESTABLISHED, pEstablishedState->SessionUpdateReceived(&objMockISession));
}

TEST_F(EstablishedStateTest, TerminateUssiSendsInfoWithErrorCode)
{
    ON_CALL(*pMockUssiController, FormInfoRequest(_, _, _)).WillByDefault(Return(IMS_SUCCESS));
    EXPECT_CALL(objMockISipClientConnection, Send);
    EXPECT_CALL(*pMockUssiController, SetNextActionByTerminateUssi);

    CallReasonInfo objCallReasonInfo(CODE_UNSPECIFIED);
    pEstablishedState->TerminateUssi(objCallReasonInfo);
}

TEST_F(EstablishedStateTest, TerminateUssiNotSendsInfoWithErrorCode)
{
    ON_CALL(*pMockUssiController, FormInfoRequest(_, _, _)).WillByDefault(Return(IMS_FAILURE));
    EXPECT_CALL(objMockISipClientConnection, Close);
    EXPECT_CALL(*pMockUssiController, SetNextActionByTerminateUssi);

    CallReasonInfo objCallReasonInfo(CODE_UNSPECIFIED);
    pEstablishedState->TerminateUssi(objCallReasonInfo);
}

TEST_F(EstablishedStateTest, UssiTerminateJustInvokesSendTerminated)
{
    ON_CALL(*pMockUssiController, IsByeForUssi(_)).WillByDefault(Return(IMS_FALSE));
    EXPECT_CALL(objUiNotifier, SendTerminated(_)).Times(1);

    pEstablishedState->UssiTerminated(&objMockISession);
}

TEST_F(EstablishedStateTest, UssiTerminateInvokesCheckingUssiBodyAndSendTerminated)
{
    ON_CALL(*pMockUssiController, IsByeForUssi(_)).WillByDefault(Return(IMS_TRUE));
    MockISipMessage objMockISipMessage;
    ON_CALL(objMessage, GetMessage).WillByDefault(Return(&objMockISipMessage));
    SipMethod objSipMethod(SipMethod::PUBLISH);
    ON_CALL(objMessage, GetMethod).WillByDefault(ReturnRef(objSipMethod));
    UssiResult objResult(UssiNextAction::NOTHING, UssiError::CODE_NONE);
    EXPECT_CALL(*pMockUssiController, ParseUssiBodyAndCheckResult(_, _))
            .Times(1)
            .WillOnce(Return(objResult));
    EXPECT_CALL(objUiNotifier, SendTerminated(_)).Times(1);

    pEstablishedState->UssiTerminated(&objMockISession);
}

TEST_F(EstablishedStateTest, SendUssdSendsInfo)
{
    ON_CALL(*pMockUssiController, FormInfoRequest(_, _, _)).WillByDefault(Return(IMS_SUCCESS));
    EXPECT_CALL(objMockISipClientConnection, Send);

    AString strUssd;
    pEstablishedState->SendUssd(strUssd);
}

TEST_F(EstablishedStateTest, SendUssdNotSendsInfo)
{
    ON_CALL(*pMockUssiController, FormInfoRequest(_, _, _)).WillByDefault(Return(IMS_FAILURE));
    EXPECT_CALL(objMockISipClientConnection, Close);

    AString strUssd;
    pEstablishedState->SendUssd(strUssd);
}

TEST_F(EstablishedStateTest, UssiInfoReceivedInvokesJustTransactionResponseIfNotInfoMethod)
{
    MockISipServerConnection objMockISipServerConnection;
    SipMethod objSipMethod(SipMethod::PUBLISH);
    ON_CALL(objMockISipServerConnection, GetMethod()).WillByDefault(ReturnRef(objSipMethod));
    EXPECT_CALL(objMockISipServerConnection, InitResponse(SipStatusCode::SC_200));
    EXPECT_CALL(objMockISipServerConnection, Send);
    EXPECT_CALL(objMockISipServerConnection, Close);
    EXPECT_CALL(*pMockUssiController, ParseUssiBodyAndCheckResult(_, _)).Times(0);

    pEstablishedState->UssiInfoReceived(&objMockISession, &objMockISipServerConnection);
}

TEST_F(EstablishedStateTest, UssiInfoReceivedInvokesJustTransactionResponseIfNotValidInfoMethod)
{
    MockISipServerConnection objMockISipServerConnection;
    SipMethod objSipMethod(SipMethod::INFO);
    ON_CALL(objMockISipServerConnection, GetMethod()).WillByDefault(ReturnRef(objSipMethod));
    ON_CALL(*pMockUssiController, IsUssiInfoReceived(_)).WillByDefault(Return(IMS_TRUE));
    ON_CALL(*pMockUssiController, HasXmlBodyInInfo(_)).WillByDefault(Return(IMS_FALSE));
    EXPECT_CALL(objMockISipServerConnection, InitResponse(SipStatusCode::SC_469));
    EXPECT_CALL(objMockISipServerConnection, Send);
    EXPECT_CALL(objMockISipServerConnection, Close);
    EXPECT_CALL(*pMockUssiController, ParseUssiBodyAndCheckResult(_, _)).Times(0);

    pEstablishedState->UssiInfoReceived(&objMockISession, &objMockISipServerConnection);
}

TEST_F(EstablishedStateTest,
        UssiInfoReceivedInvokesTransactionResponseAndSendInfoForUssiWithErrorCode)
{
    MockISipServerConnection objMockISipServerConnection;
    SipMethod objSipMethod(SipMethod::INFO);
    ON_CALL(objMockISipServerConnection, GetMethod()).WillByDefault(ReturnRef(objSipMethod));
    ON_CALL(*pMockUssiController, IsUssiInfoReceived(_)).WillByDefault(Return(IMS_TRUE));
    ON_CALL(*pMockUssiController, HasXmlBodyInInfo(_)).WillByDefault(Return(IMS_TRUE));
    EXPECT_CALL(objMockISipServerConnection, InitResponse(SipStatusCode::SC_200));
    EXPECT_CALL(objMockISipServerConnection, Send);
    EXPECT_CALL(objMockISipServerConnection, Close);
    UssiResult objUssiResult(UssiNextAction::SEND_INFO_WITH_ERROR_CODE, UssiError::CODE_1);
    EXPECT_CALL(*pMockUssiController, ParseUssiBodyAndCheckResult(_, _))
            .Times(1)
            .WillOnce(Return(objUssiResult));
    EXPECT_CALL(*pMockUssiController, FormInfoRequest(_, IsEmptyString(), UssiError::CODE_1));

    pEstablishedState->UssiInfoReceived(&objMockISession, &objMockISipServerConnection);
}

TEST_F(EstablishedStateTest, UssiInfoReceivedInvokesTransactionResponseAndSendInfoForUssi)
{
    MockISipServerConnection objMockISipServerConnection;
    SipMethod objSipMethod(SipMethod::INFO);
    ON_CALL(objMockISipServerConnection, GetMethod()).WillByDefault(ReturnRef(objSipMethod));
    ON_CALL(*pMockUssiController, IsUssiInfoReceived(_)).WillByDefault(Return(IMS_TRUE));
    ON_CALL(*pMockUssiController, HasXmlBodyInInfo(_)).WillByDefault(Return(IMS_TRUE));
    EXPECT_CALL(objMockISipServerConnection, InitResponse(SipStatusCode::SC_200));
    EXPECT_CALL(objMockISipServerConnection, Send);
    EXPECT_CALL(objMockISipServerConnection, Close);
    UssiResult objUssiResult(UssiNextAction::SEND_INFO_WITH_NOTIFY_ELEMENT, UssiError::CODE_NONE);
    EXPECT_CALL(*pMockUssiController, ParseUssiBodyAndCheckResult(_, _))
            .Times(1)
            .WillOnce(Return(objUssiResult));
    EXPECT_CALL(*pMockUssiController, FormInfoRequest(_, IsEmptyString(), UssiError::CODE_NONE));

    pEstablishedState->UssiInfoReceived(&objMockISession, &objMockISipServerConnection);
}

TEST_F(EstablishedStateTest, UssiInfoReceivedInvokesJustTransactionResponseIfNoProperAction)
{
    MockISipServerConnection objMockISipServerConnection;
    SipMethod objSipMethod(SipMethod::INFO);
    ON_CALL(objMockISipServerConnection, GetMethod()).WillByDefault(ReturnRef(objSipMethod));
    ON_CALL(*pMockUssiController, IsUssiInfoReceived(_)).WillByDefault(Return(IMS_TRUE));
    ON_CALL(*pMockUssiController, HasXmlBodyInInfo(_)).WillByDefault(Return(IMS_TRUE));
    EXPECT_CALL(objMockISipServerConnection, InitResponse(SipStatusCode::SC_200));
    EXPECT_CALL(objMockISipServerConnection, Send);
    EXPECT_CALL(objMockISipServerConnection, Close);
    UssiResult objUssiResult(UssiNextAction::NOTHING, UssiError::CODE_NONE);
    EXPECT_CALL(*pMockUssiController, ParseUssiBodyAndCheckResult(_, _))
            .Times(1)
            .WillOnce(Return(objUssiResult));
    EXPECT_CALL(*pMockUssiController, FormInfoRequest(_, _, _)).Times(0);

    pEstablishedState->UssiInfoReceived(&objMockISession, &objMockISipServerConnection);
}

TEST_F(EstablishedStateTest, NotifyResponseToUssiInfoCallsTerminateUssiAfterInfoTransaction)
{
    UssiResult objUssiResult(
            UssiNextAction::SEND_INFO_WITH_ERROR_CODE_AND_TERMINATE, UssiError::CODE_NONE);
    EXPECT_CALL(*pMockUssiController, GetLastResult).WillOnce(Return(objUssiResult));
    EXPECT_CALL(objUiNotifier, SendStartFailed(_)).Times(1);

    pEstablishedState->NotifyResponseToUssiInfo(
            &objMockISipClientConnection, &objMockISipClientConnection);
}

TEST_F(EstablishedStateTest, NotifyResponseToUssiInfoNotInvokesTerminateUssiAfterInfoTransaction)
{
    UssiResult objUssiResult(UssiNextAction::SEND_INFO_WITH_ERROR_CODE, UssiError::CODE_NONE);
    EXPECT_CALL(*pMockUssiController, GetLastResult).WillOnce(Return(objUssiResult));
    EXPECT_CALL(objUiNotifier, SendStartFailed(_)).Times(0);

    pEstablishedState->NotifyResponseToUssiInfo(
            &objMockISipClientConnection, &objMockISipClientConnection);
}

TEST_F(EstablishedStateTest, NotifyErrorToUssiInfoInvokesTerminateUssiAfterInfoTransaction)
{
    EXPECT_CALL(objMockISipClientConnection, Close);
    UssiResult objUssiResult(
            UssiNextAction::SEND_INFO_WITH_ERROR_CODE_AND_TERMINATE, UssiError::CODE_NONE);
    EXPECT_CALL(*pMockUssiController, GetLastResult).WillOnce(Return(objUssiResult));
    EXPECT_CALL(objUiNotifier, SendStartFailed(_)).Times(1);

    AString strAny;
    pEstablishedState->NotifyErrorToUssiInfo(&objMockISipClientConnection, 0, strAny);
}

TEST_F(EstablishedStateTest, NotifyErrorToUssiInfoNotInvokesTerminateUssiAfterInfoTransaction)
{
    EXPECT_CALL(objMockISipClientConnection, Close);
    UssiResult objUssiResult(UssiNextAction::SEND_INFO_WITH_ERROR_CODE, UssiError::CODE_NONE);
    EXPECT_CALL(*pMockUssiController, GetLastResult).WillOnce(Return(objUssiResult));
    EXPECT_CALL(objUiNotifier, SendStartFailed(_)).Times(0);

    AString strAny;
    pEstablishedState->NotifyErrorToUssiInfo(&objMockISipClientConnection, 0, strAny);
}

TEST_F(EstablishedStateTest, OnReceivingNetworkToneStartedAndFailedInvokesSendHeldBy)
{
    EXPECT_CALL(objUiNotifier, SendHeldBy).Times(2);
    EXPECT_EQ(CallStateName::ESTABLISHED, pEstablishedState->OnReceivingNetworkToneStarted());
    EXPECT_EQ(CallStateName::ESTABLISHED, pEstablishedState->OnReceivingNetworkToneFailed());
}

TEST_F(EstablishedStateTest,
        UpdatePushesPendingOperationDuringRefreshOrTimerForDelayingUpdateIsActive)
{
    EXPECT_CALL(objMockISession, IsSessionRefreshInProgress)
            .Times(2)
            .WillOnce(Return(IMS_TRUE))
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(objTimerWrapper, IsActive(MtcCallState::TIMER_DELAY_UPDATE_AFTER_CONNECTED))
            .Times(1)
            .WillOnce(Return(IMS_TRUE));
    TestMtcPendingOperationHolder objPendingOperationHolder;
    ON_CALL(objMockCallContext, GetPendingOperationHolder)
            .WillByDefault(ReturnRef(objPendingOperationHolder));

    EXPECT_CALL(objPendingOperationHolder.GetMock(), Update(CallType::VT, objMediaInfo)).Times(2);

    pEstablishedState->Update(CallType::VT, objMediaInfo);
    pEstablishedState->Update(CallType::VT, objMediaInfo);
}

TEST_F(EstablishedStateTest, UpdateDoesNotRefreshConvertRemoteResponseTimer)
{
    EXPECT_CALL(objTimerWrapper, IsActive(MtcCallState::TIMER_DELAY_UPDATE_AFTER_CONNECTED))
            .WillRepeatedly(Return(IMS_FALSE));
    EXPECT_CALL(objMockMtcSession, Update(UpdateType::SESSION, IMS_FALSE, SipMethod::INVITE))
            .Times(3)
            .WillRepeatedly(Return(IMS_SUCCESS));
    EXPECT_CALL(objTimerWrapper, IsActive(MtcCallState::TIMER_CONVERT_REMOTE_RESPONSE))
            .Times(3)
            .WillOnce(Return(IMS_FALSE))
            .WillOnce(Return(IMS_TRUE))
            .WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(objTimerWrapper, Start(MtcCallState::TIMER_CONVERT_REMOTE_RESPONSE, _)).Times(1);

    pEstablishedState->Update(CallType::VT, objMediaInfo);
    pEstablishedState->Update(CallType::VT, objMediaInfo);
    pEstablishedState->Update(CallType::VT, objMediaInfo);
}

TEST_F(EstablishedStateTest,
        HandleAosDisconnectedWithRegTerminatingInvokesTerminateWithLocalServiceUnavailable)
{
    IMS_UINT32 nAosReason = ImsAosReason::REG_TERMINATING;

    ON_CALL(objService, GetSrvccState).WillByDefault(Return(SrvccState::IDLE));

    MockEpsFallbackTrigger* pEpsFbTrigger = new MockEpsFallbackTrigger(objMockCallContext);
    ON_CALL(objMockCallContext, GetEpsFallbackTrigger).WillByDefault(ReturnRef(*pEpsFbTrigger));
    ON_CALL(*pEpsFbTrigger, IsWaitingEpsFallbackForNoResponse).WillByDefault(Return(IMS_FALSE));
    ON_CALL(*pEpsFbTrigger, IsWaitingEpsFallbackForNoTrigger).WillByDefault(Return(IMS_FALSE));
    ON_CALL(*pMockConfigurationManager, IsRegistrationDisconnectReasonToIgnore(nAosReason))
            .WillByDefault(Return(IMS_FALSE));

    const CallReasonInfo objReasonInfo(CODE_LOCAL_SERVICE_UNAVAILABLE);
    EXPECT_CALL(objMockMtcSession, Terminate(IMS_TRUE, objReasonInfo)).Times(1);
    EXPECT_CALL(objUiNotifier, SendTerminated(objReasonInfo)).Times(1);

    EXPECT_EQ(CallStateName::TERMINATING,
            pEstablishedState->OnAosStateChanged(MtcAosState::DISCONNECTED, nAosReason));

    delete pEpsFbTrigger;
}

TEST_F(EstablishedStateTest, HoldPushesPendingOperation)
{
    EXPECT_CALL(objMockISession, IsSessionRefreshInProgress)
            .Times(2)
            .WillOnce(Return(IMS_TRUE))
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(objTimerWrapper, IsActive(MtcCallState::TIMER_DELAY_UPDATE_AFTER_CONNECTED))
            .Times(1)
            .WillOnce(Return(IMS_TRUE));
    TestMtcPendingOperationHolder objPendingOperationHolder;
    ON_CALL(objMockCallContext, GetPendingOperationHolder)
            .WillByDefault(ReturnRef(objPendingOperationHolder));

    EXPECT_CALL(objPendingOperationHolder.GetMock(), Hold(objMediaInfo)).Times(2);

    pEstablishedState->Hold(objMediaInfo);
    pEstablishedState->Hold(objMediaInfo);
}

TEST_F(EstablishedStateTest, HoldInvokesSendHeldWithAudioDirectionActive)
{
    ON_CALL(objMockISession, IsSessionRefreshInProgress).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objTimerWrapper, IsActive(MtcCallState::TIMER_DELAY_UPDATE_AFTER_CONNECTED))
            .WillByDefault(Return(IMS_FALSE));
    objMediaInfo.eAudioDirection = DIRECTION_INACTIVE;

    EXPECT_CALL(objMockCallContext, SetHeldByMe(IMS_TRUE));
    EXPECT_CALL(objUiNotifier, SendHeld());

    pEstablishedState->Hold(objMediaInfo);
}

TEST_F(EstablishedStateTest, HoldInvokesHandleUpdate)
{
    ON_CALL(objMockISession, IsSessionRefreshInProgress).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objTimerWrapper, IsActive(MtcCallState::TIMER_DELAY_UPDATE_AFTER_CONNECTED))
            .WillByDefault(Return(IMS_FALSE));

    EXPECT_CALL(objMockMtcSession, Update(UpdateType::HOLD, IMS_FALSE, SipMethod::INVITE));

    EXPECT_EQ(CallStateName::UPDATING, pEstablishedState->Hold(objMediaInfo));
    EXPECT_EQ(UpdateType::HOLD, pUpdatingInfo->GetRequestingType());
}

TEST_F(EstablishedStateTest, ResumePushesPendingOperation)
{
    EXPECT_CALL(objMockISession, IsSessionRefreshInProgress)
            .Times(2)
            .WillOnce(Return(IMS_TRUE))
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(objTimerWrapper, IsActive(MtcCallState::TIMER_DELAY_UPDATE_AFTER_CONNECTED))
            .Times(1)
            .WillOnce(Return(IMS_TRUE));
    TestMtcPendingOperationHolder objPendingOperationHolder;
    ON_CALL(objMockCallContext, GetPendingOperationHolder)
            .WillByDefault(ReturnRef(objPendingOperationHolder));

    EXPECT_CALL(objPendingOperationHolder.GetMock(), Resume(objMediaInfo)).Times(2);

    pEstablishedState->Resume(objMediaInfo);
    pEstablishedState->Resume(objMediaInfo);
}

TEST_F(EstablishedStateTest, ResumeInvokesHandleUpdate)
{
    ON_CALL(objMockISession, IsSessionRefreshInProgress).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objTimerWrapper, IsActive(MtcCallState::TIMER_DELAY_UPDATE_AFTER_CONNECTED))
            .WillByDefault(Return(IMS_FALSE));

    EXPECT_CALL(objMockMtcSession, Update(UpdateType::RESUME, IMS_FALSE, SipMethod::INVITE));

    EXPECT_EQ(CallStateName::UPDATING, pEstablishedState->Resume(objMediaInfo));
    EXPECT_EQ(UpdateType::RESUME, pUpdatingInfo->GetRequestingType());
}

}  // namespace android
