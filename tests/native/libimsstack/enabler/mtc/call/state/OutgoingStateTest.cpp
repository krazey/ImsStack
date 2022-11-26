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
#include "MockIMtcCallController.h"
#include "MockIMtcService.h"
#include "call/IMtcCall.h"
#include "call/MockEpsFallbackTrigger.h"
#include "call/MockIMtcCallContext.h"
#include "call/MockIMtcSession.h"
#include "call/MockIMtcUiNotifier.h"
#include "call/MockISilentRedialHelper.h"
#include "call/MockMtcPendingOperationHolder.h"
#include "call/state/MtcCallState.h"
#include "call/state/OutgoingState.h"
#include "configuration/MockIMtcConfigurationManager.h"
#include "configuration/MtcConfigurationProxy.h"
#include "core/MockISession.h"
#include "helper/ISrvccStateListener.h"
#include "helper/MockIMtcAosConnector.h"
#include "helper/MtcTimerWrapper.h"
#include "media/MockIMtcMediaManager.h"
#include "precondition/MockIMtcPreconditionManager.h"
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;

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
    MockIMtcMediaManager objMockMediaManager;
    MockIMtcPreconditionManager objPreconditionManager;
    MockIMtcCallController objController;
    MockISilentRedialHelper objRedialHelper;
    CallInfo objCallInfo;
    MtcTimerWrapper objTimer;
    MockIMtcUiNotifier objNotifier;

protected:
    virtual void SetUp() override
    {
        pConfigurationManager = new MockIMtcConfigurationManager();
        pConfigurationProxy = new MtcConfigurationProxy(pConfigurationManager);
        ON_CALL(objCallContext, GetConfigurationProxy)
                .WillByDefault(ReturnRef(*pConfigurationProxy));

        ON_CALL(objCallContext, GetCallController).WillByDefault(ReturnRef(objController));
        ON_CALL(objController, GetRedialHelper).WillByDefault(ReturnRef(objRedialHelper));
        ON_CALL(objCallContext, GetUiNotifier).WillByDefault(ReturnRef(objNotifier));

        ON_CALL(objCallContext, GetSession()).WillByDefault(Return(&objMtcSession));
        ON_CALL(objCallContext, GetCallInfo).WillByDefault(ReturnRef(objCallInfo));
        ON_CALL(objCallContext, GetService).WillByDefault(ReturnRef(objService));
        ON_CALL(objCallContext, GetTimer).WillByDefault(ReturnRef(objTimer));
        ON_CALL(objCallContext, GetMediaManager).WillByDefault(ReturnRef(objMockMediaManager));

        ON_CALL(objService, GetAosConnector).WillByDefault(Return(&objAosConnector));

        ON_CALL(objMtcSession, GetISession).WillByDefault(ReturnRef(objSession));

        ON_CALL(objCallContext, GetPreconditionManager)
                .WillByDefault(ReturnRef(objPreconditionManager));

        pOutgoingState = new OutgoingState(objCallContext);
    }

    virtual void TearDown() override
    {
        delete pOutgoingState;
        delete pConfigurationProxy;
    }
};

TEST_F(OutgoingStateTest, SessionPRAckDeliveryFailedIgnoredIfConfigOn)
{
    ON_CALL(*pConfigurationManager, IsIgnorePrackDeliveryFailure).WillByDefault(Return(IMS_TRUE));

    EXPECT_EQ(CallStateName::OUTGOING, pOutgoingState->SessionPRAckDeliveryFailed(&objSession));
}

TEST_F(OutgoingStateTest, HandldB1TimerIsNotHandledIfNoUserTerminateCase)
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
            .WillByDefault(Return(2));

    pOutgoingState->OnTimerExpired(MtcCallState::TIMER_MO_100_WAIT);
    const CallReasonInfo objReason(CODE_USER_TERMINATED);
    pOutgoingState->Terminate(objReason);
}

TEST_F(OutgoingStateTest, HandleB1TimerIsHandled)
{
    EXPECT_CALL(objAosConnector, Control).Times(1);

    ON_CALL(objService, IsWlanIpCanType).WillByDefault(Return(IMS_FALSE));
    ON_CALL(*pConfigurationManager, GetPolicyForTcallTimerExpiryOfVolteCall)
            .WillByDefault(Return(1));

    pOutgoingState->OnTimerExpired(MtcCallState::TIMER_MO_100_WAIT);
    const CallReasonInfo objReason(CODE_USER_TERMINATED);
    pOutgoingState->Terminate(objReason);
}

TEST_F(OutgoingStateTest, OnMediaFailed)
{
    EXPECT_CALL(objPreconditionManager, FormPreconditionSdp).Times(0);

    MockIMtcSession* pMtcSession = new MockIMtcSession();
    ON_CALL(objCallContext, GetSession(_)).WillByDefault(Return(pMtcSession));

    EXPECT_CALL(*pMtcSession, Terminate(_, CallReasonInfo(CODE_MEDIA_INIT_FAILED))).Times(1);

    EXPECT_CALL(objSession, GetPreviousResponse(_)).Times(0);

    pOutgoingState->OnMediaFailed(CallReasonInfo(CODE_MEDIA_INIT_FAILED));

    delete pMtcSession;
}

TEST_F(OutgoingStateTest, OnIpcanChangedPushesPendingOperation)
{
    MockMtcPendingOperationHolder objPendingOperationHolder;
    ON_CALL(objCallContext, GetPendingOperationHolder)
            .WillByDefault(ReturnRef(objPendingOperationHolder));

    IMS_UINT32 eIpcan = 1;
    EXPECT_CALL(objPendingOperationHolder, PushPendingOperation(_));

    pOutgoingState->OnIpcanChanged(eIpcan);
}

TEST_F(OutgoingStateTest, SendUpdateBySrvccByCanceled)
{
    ON_CALL(objMockMediaManager, GetNegotiationState(_))
            .WillByDefault(Return(NegotiationState::STATE_NEGOTIATED));
    EXPECT_CALL(objMtcSession, SendEarlyUpdate(UpdateType::SRVCC_RECOVERED_CANCEL)).Times(1);

    EXPECT_EQ(CallStateName::OUTGOING, pOutgoingState->OnSrvccStateUpdated(SrvccState::CANCELED));
}

TEST_F(OutgoingStateTest, SendUpdateBySrvccByFailed)
{
    ON_CALL(objMockMediaManager, GetNegotiationState(_))
            .WillByDefault(Return(NegotiationState::STATE_NEGOTIATED));
    EXPECT_CALL(objMtcSession, SendEarlyUpdate(UpdateType::SRVCC_RECOVERED_FAILURE)).Times(1);

    EXPECT_EQ(CallStateName::OUTGOING, pOutgoingState->OnSrvccStateUpdated(SrvccState::FAILED));
}

TEST_F(OutgoingStateTest, HandleSilentRedialInvokesRedial)
{
    // TODO: test StartFailed by With SilentRedial
}

TEST_F(OutgoingStateTest, HandleAosConnectedDoesNothingIfNoWatchdogTimer)
{
    ON_CALL(*pConfigurationManager, GetEpsFallbackWatchdogTime).WillByDefault(Return(-1));

    EXPECT_EQ(
            CallStateName::OUTGOING, pOutgoingState->OnAosStateChanged(MtcAosState::CONNECTED, 0));
}

TEST_F(OutgoingStateTest, HandleAosConnectedInvokesEpsFallbackApis)
{
    MockEpsFallbackTrigger objEpsFbTrigger(objCallContext);
    ON_CALL(objCallContext, GetEpsFallbackTrigger).WillByDefault(ReturnRef(objEpsFbTrigger));
    ON_CALL(*pConfigurationManager, GetEpsFallbackWatchdogTime).WillByDefault(Return(-1));
    EXPECT_EQ(
            CallStateName::OUTGOING, pOutgoingState->OnAosStateChanged(MtcAosState::CONNECTED, 0));

    ON_CALL(*pConfigurationManager, GetEpsFallbackWatchdogTime).WillByDefault(Return(6000));
    ON_CALL(objEpsFbTrigger, IsWaitingEpsFallbackForNoResponse).WillByDefault(Return(IMS_FALSE));
    EXPECT_EQ(
            CallStateName::OUTGOING, pOutgoingState->OnAosStateChanged(MtcAosState::CONNECTED, 0));

    ON_CALL(objEpsFbTrigger, IsWaitingEpsFallbackForNoResponse).WillByDefault(Return(IMS_TRUE));

    EXPECT_CALL(objRedialHelper, Redial).Times(1).WillOnce(Return(IMS_SUCCESS));
    EXPECT_CALL(objEpsFbTrigger, OnEpsFallbackCompleted);
    EXPECT_EQ(CallStateName::IDLE, pOutgoingState->OnAosStateChanged(MtcAosState::CONNECTED, 0));
}

TEST_F(OutgoingStateTest, OnReceivingMediaDataStartedStopsUdpKeepAliveSender)
{
    // TODO: add unit test.
}
