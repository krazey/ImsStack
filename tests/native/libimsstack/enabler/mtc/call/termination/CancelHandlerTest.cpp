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
#include "ImsList.h"
#include "ImsTypeDef.h"
#include "call/MockIMtcCallContext.h"
#include "call/termination/CancelHandler.h"
#include "core/MockIMessage.h"
#include "sipcore/ISipHeader.h"
#include "sipcore/MockISipMessage.h"
#include "utility/IMessageUtils.h"
#include "utility/MockIMessageUtils.h"
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;

LOCAL AString SIP_PROTOCOL = "SIP";
LOCAL AString NON_SIP_PROTOCOL = "Q.850";

class CancelHandlerTest : public ::testing::Test
{
public:
    inline CancelHandlerTest() :
            objHandler(objContext)
    {
    }

    MockIMtcCallContext objContext;
    MockISipMessage objSipMessage;
    MockIMessage objMessage;
    MockIMessageUtils objMessageUtils;
    CancelHandler objHandler;

protected:
    virtual void SetUp() override
    {
        ON_CALL(objContext, GetMessageUtils).WillByDefault(ReturnRef(objMessageUtils));

        ON_CALL(objMessage, GetMessage).WillByDefault(Return(&objSipMessage));
    }

    virtual void TearDown() override {}

    void SetUpReasonHeader(
            IN const AString& strProtocol, IN IMS_SINT32 nCause, IN const AString& strText)
    {
        ReasonHeaderValue objValue;
        objValue.nCause = nCause;
        objValue.strText = strText;
        ON_CALL(objMessageUtils, GetCauseAndTextFromReasonHeader(&objMessage, strProtocol))
                .WillByDefault(Return(objValue));
    }
};

TEST_F(CancelHandlerTest, HandleMessageWithNoReasonReturnsDefaultReason)
{
    SetUpReasonHeader(SIP_PROTOCOL, -1, "");

    EXPECT_EQ(CallReasonInfo(CODE_USER_TERMINATED_BY_REMOTE), objHandler.Handle(objMessage));
}

TEST_F(CancelHandlerTest, HandleMessageReturnsDefaultReason)
{
    SetUpReasonHeader(SIP_PROTOCOL, 999, "\"any_text\"");

    EXPECT_EQ(CallReasonInfo(CODE_USER_TERMINATED_BY_REMOTE), objHandler.Handle(objMessage));
}

TEST_F(CancelHandlerTest, HandleMessageWith200CallCompletedReturnsAnsweredElsewhere)
{
    SetUpReasonHeader(SIP_PROTOCOL, 200, "\"call completed elsewhere\"");

    EXPECT_EQ(CallReasonInfo(CODE_ANSWERED_ELSEWHERE), objHandler.Handle(objMessage));
}

TEST_F(CancelHandlerTest, HandleMessageWith600BusyEverywhereReturnsRejectedElsewhere)
{
    SetUpReasonHeader(SIP_PROTOCOL, 600, "\"busy everywhere\"");

    EXPECT_EQ(CallReasonInfo(CODE_REJECTED_ELSEWHERE), objHandler.Handle(objMessage));
}

TEST_F(CancelHandlerTest, HandleMessageWith603DeclinedReturnsRejectedElsewhere)
{
    SetUpReasonHeader(SIP_PROTOCOL, 603, "\"declined\"");

    EXPECT_EQ(CallReasonInfo(CODE_REJECTED_ELSEWHERE), objHandler.Handle(objMessage));
}

TEST_F(CancelHandlerTest, HandleMessageWithCallCompletedVzwReturnsAnsweredElsewhere)
{
    SetUpReasonHeader(SIP_PROTOCOL, 999, "\"Call Completion Elsewhere\"");

    EXPECT_EQ(CallReasonInfo(CODE_ANSWERED_ELSEWHERE), objHandler.Handle(objMessage));
}

TEST_F(CancelHandlerTest, HandleMessageWithBusyEverywhereVzwReturnsRejectedElsewhere)
{
    SetUpReasonHeader(SIP_PROTOCOL, 999, "\"Another device sent All Devices Busy response\"");

    EXPECT_EQ(CallReasonInfo(CODE_REJECTED_ELSEWHERE), objHandler.Handle(objMessage));
}

TEST_F(CancelHandlerTest, HandleMessageCheckReasonTextCaseInsensitive)
{
    SetUpReasonHeader(SIP_PROTOCOL, 603, "\"DECLINED\"");

    EXPECT_EQ(CallReasonInfo(CODE_REJECTED_ELSEWHERE), objHandler.Handle(objMessage));
}

TEST_F(CancelHandlerTest, NonSipProtocolReasonValueIsNotUsed)
{
    SetUpReasonHeader(NON_SIP_PROTOCOL, 200, "\"call completed elsewhere\"");
    EXPECT_EQ(CallReasonInfo(CODE_USER_TERMINATED_BY_REMOTE), objHandler.Handle(objMessage));

    SetUpReasonHeader(SIP_PROTOCOL, 200, "\"call completed elsewhere\"");
    EXPECT_EQ(CallReasonInfo(CODE_ANSWERED_ELSEWHERE), objHandler.Handle(objMessage));
}
