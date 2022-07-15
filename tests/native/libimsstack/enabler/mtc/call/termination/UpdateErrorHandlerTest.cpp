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

#include <array>
#include <gtest/gtest.h>
#include "ImsTypeDef.h"
#include "call/MockIMtcCallContext.h"
#include "call/termination/UpdateErrorHandler.h"
#include "core/MockIMessage.h"
#include "sipcore/MockISipMessage.h"
#include "sipcore/SipStatusCode.h"

using ::testing::Return;
using ::testing::ReturnRef;

class UpdateErrorHandlerTest : public ::testing::Test
{
public:
    MockISipMessage objSipMessage;
    MockIMessage objMessage;
    MockIMtcCallContext objContext;
    CallInfo objCallInfo;
    UpdateErrorHandler* pHandler;

protected:
    virtual void SetUp() override
    {
        ON_CALL(objMessage, GetMessage)
                .WillByDefault(Return(&objSipMessage));

        ON_CALL(objContext, GetCallInfo)
                .WillByDefault(ReturnRef(objCallInfo));

        pHandler = new UpdateErrorHandler(objContext);
    }

    virtual void TearDown() override {}
};

TEST_F(UpdateErrorHandlerTest, HandleNullMessageReturnsServerError)
{
    EXPECT_EQ(CallReasonInfo(CODE_SIP_SERVER_ERROR), pHandler->Handle(IMS_NULL));
}

TEST_F(UpdateErrorHandlerTest, Handle3xxMessageReturnsServerError)
{
    for (IMS_SINT32 nStatusCode = SipStatusCode::SC_300; nStatusCode < SipStatusCode::SC_400;
            nStatusCode++)
    {
        ON_CALL(objMessage, GetStatusCode)
                .WillByDefault(Return(nStatusCode));

        EXPECT_EQ(CallReasonInfo(CODE_SIP_SERVER_ERROR, nStatusCode),
                pHandler->Handle(&objMessage));
    }
}

TEST_F(UpdateErrorHandlerTest, Handle4xxMessageReturnsTerminatedByRemote)
{
    std::array<IMS_SINT32, 11> objStatusCodes = {
            SipStatusCode::SC_404,
            SipStatusCode::SC_405,
            SipStatusCode::SC_410,
            SipStatusCode::SC_416,
            SipStatusCode::SC_480,
            SipStatusCode::SC_481,
            SipStatusCode::SC_482,
            SipStatusCode::SC_483,
            SipStatusCode::SC_484,
            SipStatusCode::SC_485,
            SipStatusCode::SC_489,
    };

    for (IMS_SINT32 nStatusCode : objStatusCodes)
    {
        ON_CALL(objMessage, GetStatusCode)
                .WillByDefault(Return(nStatusCode));

        EXPECT_EQ(CallReasonInfo(CODE_USER_TERMINATED_BY_REMOTE, nStatusCode),
                pHandler->Handle(&objMessage));
    }
}

TEST_F(UpdateErrorHandlerTest, Handle400MessageReturnsServerError)
{
    IMS_SINT32 nStatusCode = SipStatusCode::SC_400;
    ON_CALL(objMessage, GetStatusCode)
            .WillByDefault(Return(nStatusCode));

    EXPECT_EQ(CallReasonInfo(CODE_SIP_SERVER_ERROR, nStatusCode), pHandler->Handle(&objMessage));
}

TEST_F(UpdateErrorHandlerTest, Handle491MessageReturnsRequestPendingForMo)
{
    IMS_SINT32 nStatusCode = SipStatusCode::SC_491;
    ON_CALL(objMessage, GetStatusCode)
            .WillByDefault(Return(nStatusCode));

    objCallInfo.ePeerType = PeerType::MO;

    EXPECT_EQ(CODE_SIP_REQUEST_PENDING, pHandler->Handle(&objMessage).nCode);
}

TEST_F(UpdateErrorHandlerTest, Handle491MessageReturnsRequestPendingForMt)
{
    IMS_SINT32 nStatusCode = SipStatusCode::SC_491;
    ON_CALL(objMessage, GetStatusCode)
            .WillByDefault(Return(nStatusCode));

    objCallInfo.ePeerType = PeerType::MT;

    EXPECT_EQ(CODE_SIP_REQUEST_PENDING, pHandler->Handle(&objMessage).nCode);
}

TEST_F(UpdateErrorHandlerTest, Handle5xxMessageReturnsTerminatedByRemote)
{
    std::array<IMS_SINT32, 2> objStatusCodes = {
            SipStatusCode::SC_501,
            SipStatusCode::SC_502,
    };

    for (IMS_SINT32 nStatusCode : objStatusCodes)
    {
        ON_CALL(objMessage, GetStatusCode)
                .WillByDefault(Return(nStatusCode));

        EXPECT_EQ(CallReasonInfo(CODE_USER_TERMINATED_BY_REMOTE, nStatusCode),
                pHandler->Handle(&objMessage));
    }
}

TEST_F(UpdateErrorHandlerTest, Handle500MessageReturnsServerError)
{
    IMS_SINT32 nStatusCode = SipStatusCode::SC_500;
    ON_CALL(objMessage, GetStatusCode)
            .WillByDefault(Return(nStatusCode));

    EXPECT_EQ(CallReasonInfo(CODE_SIP_SERVER_ERROR, nStatusCode), pHandler->Handle(&objMessage));
}

TEST_F(UpdateErrorHandlerTest, Handle6xxMessageReturnsTerminatedByRemote)
{
    std::array<IMS_SINT32, 1> objStatusCodes = {
            SipStatusCode::SC_604,
    };

    for (IMS_SINT32 nStatusCode : objStatusCodes)
    {
        ON_CALL(objMessage, GetStatusCode)
                .WillByDefault(Return(nStatusCode));

        EXPECT_EQ(CallReasonInfo(CODE_USER_TERMINATED_BY_REMOTE, nStatusCode),
                pHandler->Handle(&objMessage));
    }
}

TEST_F(UpdateErrorHandlerTest, Handle600MessageReturnsServerError)
{
    IMS_SINT32 nStatusCode = SipStatusCode::SC_600;
    ON_CALL(objMessage, GetStatusCode)
            .WillByDefault(Return(nStatusCode));

    EXPECT_EQ(CallReasonInfo(CODE_SIP_SERVER_ERROR, nStatusCode), pHandler->Handle(&objMessage));
}
