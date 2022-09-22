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
#include "ImsTypeDef.h"
#include "call/termination/EarlyUpdateErrorHandler.h"
#include "core/MockIMessage.h"
#include "sipcore/MockISipMessage.h"
#include "sipcore/SipStatusCode.h"

using ::testing::Return;

class EarlyUpdateErrorHandlerTest : public ::testing::Test
{
public:
    MockISipMessage objSipMessage;
    MockIMessage objMessage;
    EarlyUpdateErrorHandler objHandler;

protected:
    virtual void SetUp() override
    {
        ON_CALL(objMessage, GetMessage)
                .WillByDefault(Return(&objSipMessage));
    }

    virtual void TearDown() override {}
};

TEST_F(EarlyUpdateErrorHandlerTest, HandleNullMessageReturnsTimeout)
{
    EXPECT_EQ(CallReasonInfo(CODE_NETWORK_RESP_TIMEOUT, EXTRA_CODE_METHOD_UPDATE),
            objHandler.Handle(IMS_NULL));
}

TEST_F(EarlyUpdateErrorHandlerTest, HandleMessageWithInvalidStatusCodeReturnsTimeout)
{
    ON_CALL(objSipMessage, GetStatusCode)
            .WillByDefault(Return(SipStatusCode::SC_INVALID));

    EXPECT_EQ(CallReasonInfo(CODE_NETWORK_RESP_TIMEOUT, EXTRA_CODE_METHOD_UPDATE),
            objHandler.Handle(&objMessage));
}

TEST_F(EarlyUpdateErrorHandlerTest, Handle3xx4xx5xx6xxMessageReturnsInternalErrorWithCode)
{
    for (IMS_SINT32 nStatusCode = SipStatusCode::SC_300; nStatusCode <= SipStatusCode::SC_699;
            nStatusCode++)
    {
        ON_CALL(objMessage, GetStatusCode)
                .WillByDefault(Return(nStatusCode));

        EXPECT_EQ(CallReasonInfo(CODE_REJECT_INTERNAL_ERROR, nStatusCode),
                objHandler.Handle(&objMessage));
    }
}
