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
#include <gmock/gmock.h>
#include "call/state/AlertingState.h"
#include "call/MockIMtcCallContext.h"
#include "call/MockIMtcSession.h"
#include "call/MockIMtcUiNotifier.h"
#include "helper/ISrvccStateListener.h"
#include "media/MockIMtcMediaManager.h"
#include "precondition/MockIMtcPreconditionManager.h"
#include "MockIMtcService.h"
#include "configuration/MockIMtcConfigurationManager.h"
#include "configuration/MtcConfigurationProxy.h"
#include "utility/MessageUtil.h"
#include "core/MockIMessage.h"
#include "core/MockISession.h"
#include "sipcore/MockISipMessage.h"

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;

class AlertingStateTest : public ::testing::Test
{
public:
    AlertingState* pAlertingState;
    MockISession objMockISession;
    MockIMessage objMockIMessage;
    MockISipMessage objMockISipMessage;
    MockIMtcCallContext objMockCallContext;
    MockIMtcConfigurationManager* pMockConfigurationManager;
    MtcConfigurationProxy* pConfigurationProxy;
    MockIMtcService objMockService;
    MockIMtcMediaManager objMockMediaManager;
    MockIMtcPreconditionManager objMockPreconditionManager;
    MockIMtcSession objMtcSession;
    MockIMtcUiNotifier objUiNotifier;

protected:
    virtual void SetUp() override
    {
        ON_CALL(objMockISession, GetPreviousRequest(_)).WillByDefault(Return(&objMockIMessage));
        ON_CALL(objMockIMessage, GetMessage).WillByDefault(Return(&objMockISipMessage));

        pMockConfigurationManager = new MockIMtcConfigurationManager();
        pConfigurationProxy = new MtcConfigurationProxy(pMockConfigurationManager);
        ON_CALL(objMockCallContext, GetConfigurationProxy)
                .WillByDefault(ReturnRef(*pConfigurationProxy));

        CallInfo objCallInfo;
        ON_CALL(objMockCallContext, GetCallInfo).WillByDefault(ReturnRef(objCallInfo));

        ON_CALL(objMockCallContext, GetService).WillByDefault(ReturnRef(objMockService));

        ON_CALL(objMockCallContext, GetMediaManager).WillByDefault(ReturnRef(objMockMediaManager));

        ON_CALL(objMockCallContext, GetPreconditionManager)
                .WillByDefault(ReturnRef(objMockPreconditionManager));

        ON_CALL(objMockCallContext, GetSession()).WillByDefault(Return(&objMtcSession));

        ON_CALL(objMockCallContext, GetUiNotifier).WillByDefault(ReturnRef(objUiNotifier));

        ON_CALL(objMtcSession, GetISession)
                .WillByDefault(ReturnRef(objMockISession));

        pAlertingState = new AlertingState(objMockCallContext);
    }

    virtual void TearDown() override
    {
        delete pAlertingState;
        delete pConfigurationProxy;
    }
};

TEST_F(AlertingStateTest, AcceptIsCalledWhenUpdateIsNotSentBySrvcc)
{
    ON_CALL(objMockISession, GetPreviousResponse(_)).WillByDefault(Return(&objMockIMessage));

    ON_CALL(objMockIMessage, GetState).WillByDefault(Return(IMessage::STATE_SENT));

    ImsList<AString> objReasons;
    ON_CALL(objMockISipMessage, GetHeaders(_, _)).WillByDefault(Return(objReasons));

    MockIMessage objMockNextMessage;
    ON_CALL(objMockISession, GetNextResponse).WillByDefault(Return(&objMockNextMessage));

    EXPECT_CALL(objMtcSession, Accept).Times(1);
    pAlertingState->SessionEarlyMediaUpdated(&objMockISession);
}

TEST_F(AlertingStateTest, NoAcceptIsCalledWhenUpdateIsSentBySrvcc)
{
    ON_CALL(objMockISession, GetPreviousResponse(_)).WillByDefault(Return(&objMockIMessage));

    ON_CALL(objMockIMessage, GetState).WillByDefault(Return(IMessage::STATE_SENT));

    ImsList<AString> objReasons;
    objReasons.Append(MessageUtil::STR_REASON_HANDOVER_CANCELLED);
    ON_CALL(objMockISipMessage, GetHeaders(_, _)).WillByDefault(Return(objReasons));

    EXPECT_CALL(objMtcSession, Accept).Times(0);
    pAlertingState->SessionEarlyMediaUpdated(&objMockISession);
}

TEST_F(AlertingStateTest, OnMediaFailed)
{
    EXPECT_CALL(objMockPreconditionManager, FormPreconditionSdp).Times(0);

    EXPECT_CALL(objMtcSession, Reject(CallReasonInfo(CODE_MEDIA_INIT_FAILED))).Times(1);

    pAlertingState->OnMediaFailed(CallReasonInfo(CODE_MEDIA_INIT_FAILED));
}

TEST_F(AlertingStateTest, SendUpdateBySrvccByCanceled)
{
    ON_CALL(objMockMediaManager, GetNegotiationState(_))
            .WillByDefault(Return(NegotiationState::STATE_NEGOTIATED));
    EXPECT_CALL(objMtcSession, SendEarlyUpdate(UpdateType::SRVCC_RECOVERED_CANCEL)).Times(1);

    EXPECT_EQ(CallStateName::ALERTING, pAlertingState->OnSrvccStateUpdated(SrvccState::CANCELED));
}

TEST_F(AlertingStateTest, SendUpdateBySrvccByFailed)
{
    ON_CALL(objMockMediaManager, GetNegotiationState(_))
            .WillByDefault(Return(NegotiationState::STATE_NEGOTIATED));
    EXPECT_CALL(objMtcSession, SendEarlyUpdate(UpdateType::SRVCC_RECOVERED_FAILURE)).Times(1);

    EXPECT_EQ(CallStateName::ALERTING, pAlertingState->OnSrvccStateUpdated(SrvccState::FAILED));
}
