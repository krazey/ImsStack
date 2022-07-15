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
#include "call/termination/CancelHandler.h"
#include "core/MockIMessage.h"
#include "sipcore/MockISipMessage.h"
#include "sipcore/SipHeaderName.h"
#include "sipcore/ISipHeader.h"

using ::testing::Return;

class CancelHandlerTest : public ::testing::Test
{
public:
    MockISipMessage objSipMessage;
    MockIMessage objMessage;
    CancelHandler objHandler;

protected:
    virtual void SetUp() override
    {
        ON_CALL(objMessage, GetMessage)
                .WillByDefault(Return(&objSipMessage));
    }

    virtual void TearDown() override {}
};

TEST_F(CancelHandlerTest, HandleMessageWithNoReasonReturnsDefaultReason)
{
    ImsList<AString> lstReasonHeaders;
    AString strReasonHeaderName(SipHeaderName::REASON);
    ON_CALL(objSipMessage, GetHeaders(ISipHeader::UNKNOWN, strReasonHeaderName))
            .WillByDefault(Return(lstReasonHeaders));

    EXPECT_EQ(CallReasonInfo(CODE_USER_TERMINATED_BY_REMOTE), objHandler.Handle(objMessage));
}

TEST_F(CancelHandlerTest, HandleMessageReturnsDefaultReason)
{
    AString strReason = "SIP;cause=999;text=\"any_text\"";

    ImsList<AString> lstReasonHeaders;
    lstReasonHeaders.Append(strReason);
    AString strReasonHeaderName(SipHeaderName::REASON);
    ON_CALL(objSipMessage, GetHeaders(ISipHeader::UNKNOWN, strReasonHeaderName))
            .WillByDefault(Return(lstReasonHeaders));

    EXPECT_EQ(CallReasonInfo(CODE_USER_TERMINATED_BY_REMOTE), objHandler.Handle(objMessage));
}

TEST_F(CancelHandlerTest, HandleMessageWith200CallCompletedReturnsAnsweredElsewhere)
{
    AString strReason = "SIP;cause=200;text=\"call completed elsewhere\"";

    ImsList<AString> lstReasonHeaders;
    lstReasonHeaders.Append(strReason);
    AString strReasonHeaderName(SipHeaderName::REASON);
    ON_CALL(objSipMessage, GetHeaders(ISipHeader::UNKNOWN, strReasonHeaderName))
            .WillByDefault(Return(lstReasonHeaders));

    EXPECT_EQ(CallReasonInfo(CODE_ANSWERED_ELSEWHERE), objHandler.Handle(objMessage));
}

TEST_F(CancelHandlerTest, HandleMessageWith600BusyEverywhereReturnsRejectedElsewhere)
{
    AString strReason = "SIP;cause=600;text=\"busy everywhere\"";

    ImsList<AString> lstReasonHeaders;
    lstReasonHeaders.Append(strReason);
    AString strReasonHeaderName(SipHeaderName::REASON);
    ON_CALL(objSipMessage, GetHeaders(ISipHeader::UNKNOWN, strReasonHeaderName))
            .WillByDefault(Return(lstReasonHeaders));

    EXPECT_EQ(CallReasonInfo(CODE_REJECTED_ELSEWHERE), objHandler.Handle(objMessage));
}

TEST_F(CancelHandlerTest, HandleMessageWith603DeclinedReturnsRejectedElsewhere)
{
    AString strReason = "SIP;cause=603;text=\"declined\"";

    ImsList<AString> lstReasonHeaders;
    lstReasonHeaders.Append(strReason);
    AString strReasonHeaderName(SipHeaderName::REASON);
    ON_CALL(objSipMessage, GetHeaders(ISipHeader::UNKNOWN, strReasonHeaderName))
            .WillByDefault(Return(lstReasonHeaders));

    EXPECT_EQ(CallReasonInfo(CODE_REJECTED_ELSEWHERE), objHandler.Handle(objMessage));
}

TEST_F(CancelHandlerTest, HandleMessageCheckReasonTextCaseInsensitive)
{
    AString strReason = "SIP;cause=603;text=\"DECLINED\"";

    ImsList<AString> lstReasonHeaders;
    lstReasonHeaders.Append(strReason);
    AString strReasonHeaderName(SipHeaderName::REASON);
    ON_CALL(objSipMessage, GetHeaders(ISipHeader::UNKNOWN, strReasonHeaderName))
            .WillByDefault(Return(lstReasonHeaders));

    EXPECT_EQ(CallReasonInfo(CODE_REJECTED_ELSEWHERE), objHandler.Handle(objMessage));
}
