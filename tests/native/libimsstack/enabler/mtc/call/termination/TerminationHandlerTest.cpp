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
#include "AString.h"
#include "ImsList.h"
#include "ImsTypeDef.h"
#include "call/termination/TerminationHandler.h"
#include "core/ISession.h"
#include "core/MockIMessage.h"
#include "core/MockISession.h"

using ::testing::Return;

class TerminationHandlerTest : public ::testing::Test
{
public:
    MockISession objSession;
    TerminationHandler objHandler;

protected:
    virtual void SetUp() override {}

    virtual void TearDown() override {}
};

TEST_F(TerminationHandlerTest, HandleSessionTerminatedInvalidReturnsTerminatedByRemote)
{
    IMS_SINT32 nReason = ISession::TERMINATION_REASON_INVALID;
    ON_CALL(objSession, GetTerminationReason)
            .WillByDefault(Return(nReason));

    EXPECT_EQ(CallReasonInfo(CODE_USER_TERMINATED_BY_REMOTE, nReason),
            objHandler.Handle(objSession));
}

TEST_F(TerminationHandlerTest, HandleSessionTerminatedUnknownReturnsTerminatedByRemote)
{
    IMS_SINT32 nReason = ISession::TERMINATION_REASON_UNKNOWN;
    ON_CALL(objSession, GetTerminationReason)
            .WillByDefault(Return(nReason));

    EXPECT_EQ(CallReasonInfo(CODE_USER_TERMINATED_BY_REMOTE, nReason),
            objHandler.Handle(objSession));
}

TEST_F(TerminationHandlerTest, HandleSessionTerminatedUserActionReturnsUserTerminated)
{
    IMS_SINT32 nReason = ISession::TERMINATION_REASON_USER_ACTION;
    ON_CALL(objSession, GetTerminationReason)
            .WillByDefault(Return(nReason));

    EXPECT_EQ(CallReasonInfo(CODE_USER_TERMINATED, nReason), objHandler.Handle(objSession));
}

TEST_F(TerminationHandlerTest, HandleSessionTerminatedRemoteActionReturnsTerminatedByRemote)
{
    IMS_SINT32 nReason = ISession::TERMINATION_REASON_REMOTE_ACTION;
    ON_CALL(objSession, GetTerminationReason)
            .WillByDefault(Return(nReason));

    EXPECT_EQ(CallReasonInfo(CODE_USER_TERMINATED_BY_REMOTE, nReason),
            objHandler.Handle(objSession));
}

TEST_F(TerminationHandlerTest, HandleSessionTerminated488ReturnsRequestTimeout)
{
    IMS_SINT32 nReason = ISession::TERMINATION_REASON_REFRESH_408;
    ON_CALL(objSession, GetTerminationReason)
            .WillByDefault(Return(nReason));

    EXPECT_EQ(CallReasonInfo(CODE_SIP_REQUEST_TIMEOUT, nReason), objHandler.Handle(objSession));
}

TEST_F(TerminationHandlerTest, HandleSessionTerminated481ReturnsRequestTimeout)
{
    IMS_SINT32 nReason = ISession::TERMINATION_REASON_REFRESH_481;
    ON_CALL(objSession, GetTerminationReason)
            .WillByDefault(Return(nReason));

    EXPECT_EQ(CallReasonInfo(CODE_SIP_REQUEST_TIMEOUT, nReason), objHandler.Handle(objSession));
}

TEST_F(TerminationHandlerTest, HandleSessionTerminatedRefreshTxnTimeoutReturnsNetworkTimeout)
{
    IMS_SINT32 nReason = ISession::TERMINATION_REASON_REFRESH_TXN_TIMEOUT;
    ON_CALL(objSession, GetTerminationReason)
            .WillByDefault(Return(nReason));

    EXPECT_EQ(CallReasonInfo(CODE_NETWORK_RESP_TIMEOUT, nReason), objHandler.Handle(objSession));
}

TEST_F(TerminationHandlerTest, HandleSessionTerminatedRefreshTimeoutReturnsNetworkTimeout)
{
    IMS_SINT32 nReason = ISession::TERMINATION_REASON_REFRESH_TIMEOUT;
    ON_CALL(objSession, GetTerminationReason)
            .WillByDefault(Return(nReason));

    EXPECT_EQ(CallReasonInfo(CODE_NETWORK_RESP_TIMEOUT, nReason),
            objHandler.Handle(objSession));
}

TEST_F(TerminationHandlerTest, HandleSessionTerminatedServiceClosedReturnsServiceUnavailable)
{
    IMS_SINT32 nReason = ISession::TERMINATION_REASON_SERVICE_CLOSED;
    ON_CALL(objSession, GetTerminationReason)
            .WillByDefault(Return(nReason));

    EXPECT_EQ(CallReasonInfo(CODE_LOCAL_SERVICE_UNAVAILABLE, nReason),
            objHandler.Handle(objSession));
}

TEST_F(TerminationHandlerTest, HandleSessionReturnsDefaultReason)
{
    IMS_SINT32 nReason = ISession::TERMINATION_REASON_UNKNOWN;
    ON_CALL(objSession, GetTerminationReason)
            .WillByDefault(Return(nReason));

    EXPECT_EQ(CallReasonInfo(CODE_USER_TERMINATED_BY_REMOTE, nReason),
            objHandler.Handle(objSession));
}
