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
#include "IMessage.h"
#include "ISession.h"
#include "ImsList.h"
#include "ImsTypeDef.h"
#include "MockIMessage.h"
#include "MockISession.h"
#include "call/MockIMtcCallContext.h"
#include "call/termination/TerminationHandler.h"
#include "utility/IMessageUtils.h"
#include "utility/MockIMessageUtils.h"
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;

class TerminationHandlerTest : public ::testing::Test
{
public:
    inline TerminationHandlerTest() :
            objHandler(objContext)
    {
    }

    MockIMtcCallContext objContext;
    MockISession objSession;
    MockIMessage objMessage;
    MockIMessageUtils objMessageUtils;
    TerminationHandler objHandler;

protected:
    virtual void SetUp() override
    {
        ON_CALL(objContext, GetMessageUtils).WillByDefault(ReturnRef(objMessageUtils));
        ON_CALL(objSession, GetPreviousRequest(IMessage::SESSION_TERMINATE))
                .WillByDefault(Return(&objMessage));

        ReasonHeaderValue objValue;
        ON_CALL(objMessageUtils, GetCauseAndTextFromReasonHeader(&objMessage, _))
                .WillByDefault(Return(objValue));
    }

    virtual void TearDown() override {}
};

TEST_F(TerminationHandlerTest, HandleSessionTerminatedInvalidReturnsTerminatedByRemote)
{
    IMS_SINT32 nReason = ISession::TERMINATION_REASON_INVALID;
    ON_CALL(objSession, GetTerminationReason).WillByDefault(Return(nReason));

    EXPECT_EQ(
            CallReasonInfo(CODE_USER_TERMINATED_BY_REMOTE, nReason), objHandler.Handle(objSession));
}

TEST_F(TerminationHandlerTest, HandleSessionTerminatedUnknownReturnsTerminatedByRemote)
{
    IMS_SINT32 nReason = ISession::TERMINATION_REASON_UNKNOWN;
    ON_CALL(objSession, GetTerminationReason).WillByDefault(Return(nReason));

    EXPECT_EQ(
            CallReasonInfo(CODE_USER_TERMINATED_BY_REMOTE, nReason), objHandler.Handle(objSession));
}

TEST_F(TerminationHandlerTest, HandleSessionTerminatedUserActionReturnsUserTerminated)
{
    IMS_SINT32 nReason = ISession::TERMINATION_REASON_USER_ACTION;
    ON_CALL(objSession, GetTerminationReason).WillByDefault(Return(nReason));

    EXPECT_EQ(CallReasonInfo(CODE_USER_TERMINATED, nReason), objHandler.Handle(objSession));
}

TEST_F(TerminationHandlerTest, HandleSessionTerminatedRemoteActionReturnsTerminatedByRemote)
{
    IMS_SINT32 nReason = ISession::TERMINATION_REASON_REMOTE_ACTION;
    ON_CALL(objSession, GetTerminationReason).WillByDefault(Return(nReason));

    EXPECT_EQ(
            CallReasonInfo(CODE_USER_TERMINATED_BY_REMOTE, nReason), objHandler.Handle(objSession));
}

TEST_F(TerminationHandlerTest, HandleSessionTerminated488ReturnsRequestTimeout)
{
    IMS_SINT32 nReason = ISession::TERMINATION_REASON_REFRESH_408;
    ON_CALL(objSession, GetTerminationReason).WillByDefault(Return(nReason));

    EXPECT_EQ(CallReasonInfo(CODE_SIP_REQUEST_TIMEOUT, nReason), objHandler.Handle(objSession));
}

TEST_F(TerminationHandlerTest, HandleSessionTerminated481ReturnsRequestTimeout)
{
    IMS_SINT32 nReason = ISession::TERMINATION_REASON_REFRESH_481;
    ON_CALL(objSession, GetTerminationReason).WillByDefault(Return(nReason));

    EXPECT_EQ(CallReasonInfo(CODE_SIP_REQUEST_TIMEOUT, nReason), objHandler.Handle(objSession));
}

TEST_F(TerminationHandlerTest, HandleSessionTerminatedRefreshTxnTimeoutReturnsNetworkTimeout)
{
    IMS_SINT32 nReason = ISession::TERMINATION_REASON_REFRESH_TXN_TIMEOUT;
    ON_CALL(objSession, GetTerminationReason).WillByDefault(Return(nReason));

    EXPECT_EQ(CallReasonInfo(CODE_NETWORK_RESP_TIMEOUT, EXTRA_CODE_METHOD_UPDATE),
            objHandler.Handle(objSession));
}

TEST_F(TerminationHandlerTest, HandleSessionTerminatedRefreshTimeoutReturnsNetworkTimeout)
{
    IMS_SINT32 nReason = ISession::TERMINATION_REASON_REFRESH_TIMEOUT;
    ON_CALL(objSession, GetTerminationReason).WillByDefault(Return(nReason));

    EXPECT_EQ(CallReasonInfo(CODE_NETWORK_RESP_TIMEOUT, EXTRA_CODE_METHOD_UPDATE),
            objHandler.Handle(objSession));
}

TEST_F(TerminationHandlerTest, HandleSessionTerminatedServiceClosedReturnsServiceUnavailable)
{
    IMS_SINT32 nReason = ISession::TERMINATION_REASON_SERVICE_CLOSED;
    ON_CALL(objSession, GetTerminationReason).WillByDefault(Return(nReason));

    EXPECT_EQ(CallReasonInfo(CODE_LOCAL_NOT_REGISTERED, nReason), objHandler.Handle(objSession));
}

TEST_F(TerminationHandlerTest, HandleSessionReturnsDefaultReason)
{
    IMS_SINT32 nReason = ISession::TERMINATION_REASON_UNKNOWN;
    ON_CALL(objSession, GetTerminationReason).WillByDefault(Return(nReason));

    EXPECT_EQ(
            CallReasonInfo(CODE_USER_TERMINATED_BY_REMOTE, nReason), objHandler.Handle(objSession));
}

TEST_F(TerminationHandlerTest, HandleSessionReturnsDefaultReasonAndMessageHasReasonHeader)
{
    IMS_SINT32 nReason = ISession::TERMINATION_REASON_UNKNOWN;
    ON_CALL(objSession, GetTerminationReason).WillByDefault(Return(nReason));

    AString strReason("any reason");
    ReasonHeaderValue objValue;
    objValue.nCause = 10;
    objValue.strText = strReason;
    ON_CALL(objMessageUtils, GetCauseAndTextFromReasonHeader(&objMessage, _))
            .WillByDefault(Return(objValue));

    EXPECT_EQ(CallReasonInfo(CODE_USER_TERMINATED_BY_REMOTE, nReason, strReason),
            objHandler.Handle(objSession));
}

TEST_F(TerminationHandlerTest, HandleSessionTerminatedByCallPull)
{
    IMS_SINT32 nReason = ISession::TERMINATION_REASON_UNKNOWN;
    ON_CALL(objSession, GetTerminationReason).WillByDefault(Return(nReason));

    ReasonHeaderValue objValue;
    objValue.strText = "Call Has Been Pulled by Another Device";
    ON_CALL(objMessageUtils, GetCauseAndTextFromReasonHeader(&objMessage, _))
            .WillByDefault(Return(objValue));

    EXPECT_EQ(CallReasonInfo(CODE_CALL_END_CAUSE_CALL_PULL), objHandler.Handle(objSession));
}
