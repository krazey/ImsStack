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

#include "ImsTypeDef.h"
#include "call/IMtcCall.h"
#include "call/MockIMtcCallContext.h"
#include "call/termination/EarlyUpdateErrorHandler.h"
#include "core/MockIMessage.h"
#include "sipcore/MockISipMessage.h"
#include "sipcore/SipStatusCode.h"
#include <gtest/gtest.h>

using ::testing::Return;
using ::testing::ReturnRef;

class EarlyUpdateErrorHandlerTest : public ::testing::Test
{
public:
    MockIMtcCallContext objContext;
    MockISipMessage objSipMessage;
    MockIMessage objMessage;
    CallInfo objCallInfo;

protected:
    virtual void SetUp() override
    {
        ON_CALL(objMessage, GetMessage).WillByDefault(Return(&objSipMessage));
    }

    virtual void TearDown() override {}
};

TEST_F(EarlyUpdateErrorHandlerTest, HandleNullMessageReturnsTimeout)
{
    EXPECT_EQ(CallReasonInfo(CODE_NETWORK_RESP_TIMEOUT, EXTRA_CODE_METHOD_UPDATE),
            EarlyUpdateErrorHandler(objContext).Handle(IMS_NULL));
}

TEST_F(EarlyUpdateErrorHandlerTest, HandleMessageWithInvalidStatusCodeReturnsTimeout)
{
    ON_CALL(objSipMessage, GetStatusCode).WillByDefault(Return(SipStatusCode::SC_INVALID));

    EXPECT_EQ(CallReasonInfo(CODE_NETWORK_RESP_TIMEOUT, EXTRA_CODE_METHOD_UPDATE),
            EarlyUpdateErrorHandler(objContext).Handle(&objMessage));
}

TEST_F(EarlyUpdateErrorHandlerTest, Handle3xx4xx5xx6xxExcept491MessageReturnsInternalErrorWithCode)
{
    for (IMS_SINT32 nStatusCode = SipStatusCode::SC_300; nStatusCode <= SipStatusCode::SC_699;
            nStatusCode++)
    {
        if (nStatusCode == SipStatusCode::SC_491)
        {
            continue;
        }
        ON_CALL(objMessage, GetStatusCode).WillByDefault(Return(nStatusCode));

        EXPECT_EQ(CallReasonInfo(CODE_REJECT_INTERNAL_ERROR, nStatusCode),
                EarlyUpdateErrorHandler(objContext).Handle(&objMessage));
    }
}

TEST_F(EarlyUpdateErrorHandlerTest, Handle491MessageReturnsRequestPendingError)
{
    ON_CALL(objMessage, GetStatusCode).WillByDefault(Return(SipStatusCode::SC_491));
    ON_CALL(objContext, GetCallInfo).WillByDefault(ReturnRef(objCallInfo));

    EXPECT_EQ(CODE_SIP_REQUEST_PENDING,
            EarlyUpdateErrorHandler(objContext).Handle(&objMessage).nCode);
}
