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

#include <gtest/gtest.h>
#include "CallReasonInfo.h"
#include "MockIMtcService.h"
#include "call/IMtcCall.h"
#include "call/MockIMtcCallContext.h"
#include "call/MockIMtcSession.h"
#include "call/MtcUiNotifier.h"
#include "call/state/MtcCallState.h"
#include "call/state/OutgoingState.h"
#include "configuration/MockIMtcConfigurationManager.h"
#include "configuration/MtcConfigurationProxy.h"
#include "core/MockISession.h"
#include "helper/MockIMtcAosConnector.h"
#include "helper/MtcTimerWrapper.h"

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
    CallInfo objCallInfo;
    MtcTimerWrapper objTimer;
    MtcUiNotifier* pNotifier;

protected:
    virtual void SetUp() override
    {
        pConfigurationManager = new MockIMtcConfigurationManager();
        pConfigurationProxy = new MtcConfigurationProxy(pConfigurationManager);
        ON_CALL(objCallContext, GetConfigurationProxy)
                .WillByDefault(ReturnRef(*pConfigurationProxy));

        pNotifier = new MtcUiNotifier(objCallContext);
        ON_CALL(objCallContext, GetUiNotifier)
                .WillByDefault(ReturnRef(*pNotifier));

        ON_CALL(objCallContext, GetSession())
                .WillByDefault(Return(&objMtcSession));
        ON_CALL(objCallContext, GetCallInfo)
                .WillByDefault(ReturnRef(objCallInfo));
        ON_CALL(objCallContext, GetService)
                .WillByDefault(ReturnRef(objService));
        ON_CALL(objCallContext, GetTimer)
                .WillByDefault(ReturnRef(objTimer));

        ON_CALL(objService, GetAosConnector)
                .WillByDefault(Return(&objAosConnector));

        ON_CALL(objMtcSession, GetISession)
                .WillByDefault(ReturnRef(objSession));

        pOutgoingState = new OutgoingState(objCallContext);
    }

    virtual void TearDown() override
    {
        delete pOutgoingState;
        delete pConfigurationProxy;
        delete pNotifier;
    }
};

TEST_F(OutgoingStateTest, SessionPRAckDeliveryFailedIgnoredIfConfigOn)
{
    ON_CALL(*pConfigurationManager, IsIgnorePrackDeliveryFailure)
            .WillByDefault(Return(IMS_TRUE));

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

    ON_CALL(objService, IsWlanIpCanType)
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(*pConfigurationManager, GetPolicyForTcallTimerExpiryOfVolteCall)
            .WillByDefault(Return(2));

    pOutgoingState->OnTimerExpired(MtcCallState::TIMER_MO_100_WAIT);
    const CallReasonInfo objReason(CODE_USER_TERMINATED);
    pOutgoingState->Terminate(objReason);
}

TEST_F(OutgoingStateTest, HandleB1TimerIsHandled)
{
    EXPECT_CALL(objAosConnector, Control).Times(1);

    ON_CALL(objService, IsWlanIpCanType)
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(*pConfigurationManager, GetPolicyForTcallTimerExpiryOfVolteCall)
            .WillByDefault(Return(1));

    pOutgoingState->OnTimerExpired(MtcCallState::TIMER_MO_100_WAIT);
    const CallReasonInfo objReason(CODE_USER_TERMINATED);
    pOutgoingState->Terminate(objReason);
}
