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
#include "INetworkWatcher.h"
#include "ISession.h"
#include "MockIMessage.h"
#include "MockISession.h"
#include "call/IMtcCall.h"
#include "call/MockIMtcCallContext.h"
#include "call/MockIMtcSession.h"
#include "call/MockIMtcUiNotifier.h"
#include "call/radio/MockIMtcRadioChecker.h"
#include "call/state/TerminatingState.h"
#include "configuration/MockMtcConfigurationProxy.h"
#include "helper/MockICallStateProxy.h"
#include "helper/MockMtcTimerWrapper.h"
#include "media/MockIMtcMediaManager.h"
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::Mock;
using ::testing::Return;
using ::testing::ReturnRef;

LOCAL const CallKey ANY_CALL_KEY = 0;

class TerminatingStateTest : public ::testing::Test
{
public:
    inline TerminatingStateTest() :
            objCallContext(),
            objStateProxy(),
            objUiNotifier(),
            objMediaManager(),
            objTimerWrapper(),
            objMtcSession(),
            objSession(),
            objCallInfo(),
            objMtcRadioChecker(),
            objMessage(),
            objReason(CODE_UNSPECIFIED),
            objTerminatingState(objCallContext)
    {
    }

    MockIMtcCallContext objCallContext;
    MockICallStateProxy objStateProxy;
    MockMtcConfigurationProxy objConfigurationProxy;
    MockIMtcUiNotifier objUiNotifier;
    MockIMtcMediaManager objMediaManager;
    MockMtcTimerWrapper objTimerWrapper;
    MockIMtcSession objMtcSession;
    MockISession objSession;
    CallInfo objCallInfo;
    MockIMtcRadioChecker objMtcRadioChecker;
    MockIMessage objMessage;
    CallReasonInfo objReason;

    TerminatingState objTerminatingState;

protected:
    virtual void SetUp() override
    {
        ON_CALL(objCallContext, GetCallStateProxy).WillByDefault(ReturnRef(objStateProxy));
        ON_CALL(objCallContext, GetConfigurationProxy)
                .WillByDefault(ReturnRef(objConfigurationProxy));
        ON_CALL(objCallContext, GetUiNotifier).WillByDefault(ReturnRef(objUiNotifier));
        ON_CALL(objCallContext, GetMediaManager).WillByDefault(ReturnRef(objMediaManager));
        ON_CALL(objCallContext, GetTimer).WillByDefault(ReturnRef(objTimerWrapper));
        ON_CALL(objCallContext, GetCallInfo).WillByDefault(ReturnRef(objCallInfo));
        ON_CALL(objCallContext, GetSession()).WillByDefault(Return(&objMtcSession));
        ON_CALL(objMtcSession, GetISession).WillByDefault(ReturnRef(objSession));
        ON_CALL(objSession, GetState).WillByDefault(Return(ISession::STATE_TERMINATING));
        ON_CALL(objSession, GetPreviousResponse(IMessage::SESSION_START))
                .WillByDefault(Return(&objMessage));
        objReason.nCode = CODE_UNSPECIFIED;
        ON_CALL(objUiNotifier, GetBlockingReason).WillByDefault(ReturnRef(objReason));

        ON_CALL(objCallContext, GetCallKey).WillByDefault(Return(ANY_CALL_KEY));
        ON_CALL(objCallContext, IsEstablished).WillByDefault(Return(IMS_FALSE));
        ON_CALL(objCallContext, GetRadioChecker).WillByDefault(ReturnRef(objMtcRadioChecker));
    }

    virtual void TearDown() override {}
};

TEST_F(TerminatingStateTest, OnEnterInvokesDestroyMediaSession)
{
    EXPECT_CALL(objMediaManager, DestroyMediaSession);

    objTerminatingState.OnEnter();
}

TEST_F(TerminatingStateTest, OnEnterInvokesNotifyCallSessionReleasedIfSessionIsAlreadyTerminated)
{
    ON_CALL(objSession, GetState).WillByDefault(Return(ISession::STATE_TERMINATED));

    EXPECT_CALL(objStateProxy, NotifyCallSessionReleased(ANY_CALL_KEY, IMS_FALSE, IMS_FALSE));

    objTerminatingState.OnEnter();
}

TEST_F(TerminatingStateTest,
        OnEnterInvokesUiNotifierOnCallSessionReleasedIfSessionStateIsTerminatedAndEmergency)
{
    objCallInfo.eEmergencyType = EmergencyType::EMERGENCY_ROUTING;
    ON_CALL(objSession, GetState).WillByDefault(Return(ISession::STATE_TERMINATED));

    EXPECT_CALL(objTimerWrapper, Start(_, _)).Times(0);
    EXPECT_CALL(objUiNotifier, OnCallSessionReleased);

    objTerminatingState.OnEnter();
    Mock::VerifyAndClearExpectations(&objUiNotifier);
}

TEST_F(TerminatingStateTest, OnEnterDoesNotStartTimerIfSessionIsNotCreatedAndEmergency)
{
    objCallInfo.eEmergencyType = EmergencyType::EMERGENCY_ROUTING;
    ON_CALL(objCallContext, GetSession()).WillByDefault(Return(IMS_NULL));

    EXPECT_CALL(objTimerWrapper, Start(_, _)).Times(0);
    EXPECT_CALL(objUiNotifier, OnCallSessionReleased);

    objTerminatingState.OnEnter();
    Mock::VerifyAndClearExpectations(&objUiNotifier);
}

TEST_F(TerminatingStateTest,
        OnEnterDoesNotStartTimerIfTerminatedNotByUserWithNoResponseReceivedAndEmergency)
{
    objCallInfo.eEmergencyType = EmergencyType::EMERGENCY_ROUTING;
    ON_CALL(objSession, GetState).WillByDefault(Return(ISession::STATE_NEGOTIATING));

    ON_CALL(objSession, GetPreviousResponse(IMessage::SESSION_START))
            .WillByDefault(Return(IMS_NULL));

    EXPECT_CALL(objTimerWrapper, Start(_, _)).Times(0);
    EXPECT_CALL(objUiNotifier, OnCallSessionReleased);

    objTerminatingState.OnEnter();
    Mock::VerifyAndClearExpectations(&objUiNotifier);
}

TEST_F(TerminatingStateTest, OnEnterStartsTimerIfTerminatedByUserWithNoResponseReceivedAndEmergency)
{
    objCallInfo.eEmergencyType = EmergencyType::EMERGENCY_ROUTING;

    ON_CALL(objSession, GetPreviousResponse(IMessage::SESSION_START))
            .WillByDefault(Return(IMS_NULL));
    objReason.nCode = CODE_USER_TERMINATED;

    EXPECT_CALL(objTimerWrapper, Start(_, _));
    EXPECT_CALL(objUiNotifier, OnCallSessionReleased).Times(0);

    objTerminatingState.OnEnter();
    Mock::VerifyAndClearExpectations(&objUiNotifier);
}

TEST_F(TerminatingStateTest, OnEnterStartsTimerIfReceivedResponseAndEmergency)
{
    objCallInfo.eEmergencyType = EmergencyType::EMERGENCY_ROUTING;

    EXPECT_CALL(objTimerWrapper, Start(_, _));
    EXPECT_CALL(objUiNotifier, OnCallSessionReleased).Times(0);

    objTerminatingState.OnEnter();
    Mock::VerifyAndClearExpectations(&objUiNotifier);
}

TEST_F(TerminatingStateTest, NotifyCallSessionReleasedIsInvokedOnlyOnce)
{
    ON_CALL(objSession, GetState).WillByDefault(Return(ISession::STATE_TERMINATED));

    EXPECT_CALL(objStateProxy, NotifyCallSessionReleased(ANY_CALL_KEY, IMS_FALSE, IMS_FALSE));

    objTerminatingState.OnEnter();

    EXPECT_CALL(objStateProxy, NotifyCallSessionReleased(ANY_CALL_KEY, IMS_FALSE, IMS_FALSE))
            .Times(0);
    objTerminatingState.OnEnter();
}

TEST_F(TerminatingStateTest, DestructorNotifyCallSessionReleased)
{
    EXPECT_CALL(objStateProxy, NotifyCallSessionReleased(ANY_CALL_KEY, IMS_FALSE, IMS_FALSE));
}

TEST_F(TerminatingStateTest, SessionStartFailedInvokesNotifyCallSessionReleased)
{
    objCallInfo.eEmergencyType = EmergencyType::EMERGENCY_ROUTING;
    EXPECT_CALL(objStateProxy, NotifyCallSessionReleased(ANY_CALL_KEY, IMS_TRUE, IMS_FALSE));
    EXPECT_CALL(objUiNotifier, OnCallSessionReleased);

    objTerminatingState.SessionStartFailed(&objSession);
    Mock::VerifyAndClearExpectations(&objStateProxy);
}

TEST_F(TerminatingStateTest, SessionTerminatedInvokesNotifyCallSessionReleased)
{
    objCallInfo.eEmergencyType = EmergencyType::EMERGENCY_ROUTING;
    EXPECT_CALL(objStateProxy, NotifyCallSessionReleased(ANY_CALL_KEY, IMS_TRUE, IMS_FALSE));
    EXPECT_CALL(objUiNotifier, OnCallSessionReleased);

    objTerminatingState.SessionTerminated(&objSession);
    Mock::VerifyAndClearExpectations(&objStateProxy);
}

TEST_F(TerminatingStateTest, OnTimerExpiredInvokesNotifyCallSessionReleased)
{
    objCallInfo.eEmergencyType = EmergencyType::EMERGENCY_ROUTING;
    EXPECT_CALL(objStateProxy, NotifyCallSessionReleased(ANY_CALL_KEY, IMS_TRUE, IMS_FALSE));
    EXPECT_CALL(objUiNotifier, OnCallSessionReleased);

    objTerminatingState.OnTimerExpired(MtcCallState::TimerType::TIMER_E911_WAIT_SESSION_RELEASED);
    Mock::VerifyAndClearExpectations(&objStateProxy);
}

TEST_F(TerminatingStateTest, OnRatChangedDoesNothing)
{
    const IMS_SINT32 eAnyRat = INetworkWatcher::RADIOTECH_TYPE_UNKNOWN;
    EXPECT_EQ(CallStateName::TERMINATING, objTerminatingState.OnRatChanged(eAnyRat, eAnyRat));
}

TEST_F(TerminatingStateTest, OnEnterInvokesOnTerminatedBeforeCreatingSession)
{
    EXPECT_CALL(objMtcRadioChecker, OnTerminatedBeforeCreatingSession(_)).Times(0);

    objTerminatingState.OnEnter();

    ON_CALL(objCallContext, GetSession()).WillByDefault(Return(IMS_NULL));
    EXPECT_CALL(objMtcRadioChecker, OnTerminatedBeforeCreatingSession(_)).Times(1);

    objTerminatingState.OnEnter();
}
