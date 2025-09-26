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
#include "ISipHeader.h"
#include "ImsList.h"
#include "ImsTypeDef.h"
#include "MockIMessage.h"
#include "MockISipMessage.h"
#include "call/MockIMtcCallContext.h"
#include "call/termination/CancelHandler.h"
#include "configuration/MockMtcConfigurationProxy.h"
#include "configuration/MtcConfigurationProxy.h"
#include "utility/IMessageUtils.h"
#include "utility/MockIMessageUtils.h"
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;

class CancelHandlerTest : public ::testing::Test
{
public:
    inline CancelHandlerTest() :
            pConfigurationProxy(IMS_NULL),
            objHandler(objContext)
    {
    }

    MockIMtcCallContext objContext;
    MockISipMessage objSipMessage;
    MockIMessage objMessage;
    MockIMessageUtils objMessageUtils;
    MockMtcConfigurationProxy* pConfigurationProxy;
    CancelHandler objHandler;

protected:
    virtual void SetUp() override
    {
        ON_CALL(objContext, GetMessageUtils).WillByDefault(ReturnRef(objMessageUtils));

        ON_CALL(objMessage, GetMessage).WillByDefault(Return(&objSipMessage));
        pConfigurationProxy = new MockMtcConfigurationProxy();
        ON_CALL(objContext, GetConfigurationProxy).WillByDefault(ReturnRef(*pConfigurationProxy));
    }

    virtual void TearDown() override { delete pConfigurationProxy; }

    void SetUpPrioritizedReasonHeader(
            IN const AString& strProtocol, IN IMS_SINT32 nCause, IN const AString& strText)
    {
        ReasonHeaderValue objResult;
        objResult.strProtocol = strProtocol;
        objResult.nCause = nCause;
        objResult.strText = strText;
        ON_CALL(objMessageUtils, GetPrioritizedReasonHeader(&objMessage, _))
                .WillByDefault(Return(objResult));
    }
};

TEST_F(CancelHandlerTest, HandleMessageWithNoReasonReturnsDefaultReason)
{
    SetUpPrioritizedReasonHeader(REASON_SIP_PROTOCOL, -1, "");

    EXPECT_EQ(CallReasonInfo(CODE_USER_TERMINATED_BY_REMOTE), objHandler.Handle(objMessage));
}

TEST_F(CancelHandlerTest, HandleMessageReturnsDefaultReason)
{
    SetUpPrioritizedReasonHeader(REASON_SIP_PROTOCOL, 999, "\"any_text\"");

    EXPECT_EQ(CallReasonInfo(CODE_USER_TERMINATED_BY_REMOTE), objHandler.Handle(objMessage));
}

TEST_F(CancelHandlerTest, HandleMessageWith200CallCompletedReturnsAnsweredElsewhere)
{
    SetUpPrioritizedReasonHeader(REASON_SIP_PROTOCOL, 200, "\"call completed elsewhere\"");

    EXPECT_EQ(CallReasonInfo(CODE_ANSWERED_ELSEWHERE), objHandler.Handle(objMessage));
}

TEST_F(CancelHandlerTest, HandleMessageWith200AnyTextReturnsAnsweredElsewhere)
{
    SetUpPrioritizedReasonHeader(REASON_SIP_PROTOCOL, 200, "\"any text\"");

    EXPECT_EQ(CallReasonInfo(CODE_ANSWERED_ELSEWHERE), objHandler.Handle(objMessage));
}

TEST_F(CancelHandlerTest, HandleMessageWith600BusyEverywhereReturnsRejectedElsewhere)
{
    SetUpPrioritizedReasonHeader(REASON_SIP_PROTOCOL, 600, "\"busy everywhere\"");

    EXPECT_EQ(CallReasonInfo(CODE_REJECTED_ELSEWHERE), objHandler.Handle(objMessage));
}

TEST_F(CancelHandlerTest, HandleMessageWith600AnyTextReturnsRejectedElsewhere)
{
    SetUpPrioritizedReasonHeader(REASON_SIP_PROTOCOL, 600, "\"any text\"");

    EXPECT_EQ(CallReasonInfo(CODE_REJECTED_ELSEWHERE), objHandler.Handle(objMessage));
}

TEST_F(CancelHandlerTest, HandleMessageWith603DeclinedReturnsRejectedElsewhere)
{
    SetUpPrioritizedReasonHeader(REASON_SIP_PROTOCOL, 603, "\"declined\"");

    EXPECT_EQ(CallReasonInfo(CODE_REJECTED_ELSEWHERE), objHandler.Handle(objMessage));
}

TEST_F(CancelHandlerTest, HandleMessageWith603AnyTextReturnsRejectedElsewhere)
{
    SetUpPrioritizedReasonHeader(REASON_SIP_PROTOCOL, 603, "\"any text\"");

    EXPECT_EQ(CallReasonInfo(CODE_REJECTED_ELSEWHERE), objHandler.Handle(objMessage));
}

TEST_F(CancelHandlerTest, HandleMessageWithCallCompletedVzwReturnsAnsweredElsewhere)
{
    SetUpPrioritizedReasonHeader(REASON_SIP_PROTOCOL, 999, "\"Call Completion Elsewhere\"");

    EXPECT_EQ(CallReasonInfo(CODE_ANSWERED_ELSEWHERE), objHandler.Handle(objMessage));
}

TEST_F(CancelHandlerTest, HandleMessageWithBusyEverywhereVzwReturnsRejectedElsewhere)
{
    SetUpPrioritizedReasonHeader(
            REASON_SIP_PROTOCOL, 999, "\"Another device sent All Devices Busy response\"");

    EXPECT_EQ(CallReasonInfo(CODE_REJECTED_ELSEWHERE), objHandler.Handle(objMessage));
}

TEST_F(CancelHandlerTest, HandleMessageCheckReasonTextCaseInsensitive)
{
    SetUpPrioritizedReasonHeader(REASON_SIP_PROTOCOL, 603, "\"DECLINED\"");

    EXPECT_EQ(CallReasonInfo(CODE_REJECTED_ELSEWHERE), objHandler.Handle(objMessage));
}

TEST_F(CancelHandlerTest, NonSipProtocolReasonValueIsNotUsed)
{
    SetUpPrioritizedReasonHeader(REASON_Q850_PROTOCOL, 200, "\"call completed elsewhere\"");
    EXPECT_EQ(CallReasonInfo(CODE_USER_TERMINATED_BY_REMOTE), objHandler.Handle(objMessage));

    SetUpPrioritizedReasonHeader(REASON_SIP_PROTOCOL, 200, "\"call completed elsewhere\"");
    EXPECT_EQ(CallReasonInfo(CODE_ANSWERED_ELSEWHERE), objHandler.Handle(objMessage));
}

TEST_F(CancelHandlerTest, HandleMessageWithExtraMessageFromSIPReasonHeader)
{
    ON_CALL(*pConfigurationProxy,
            GetBoolean(ConfigVoice::KEY_ENRICH_CALLREASONINFO_WITH_REASON_HEADER_BOOL))
            .WillByDefault(Return(IMS_TRUE));
    AString strFormattedMessage = "call completed elsewhere";
    SetUpPrioritizedReasonHeader(REASON_SIP_PROTOCOL, 200, "\"call completed elsewhere\"");
    EXPECT_EQ(CallReasonInfo(CODE_ANSWERED_ELSEWHERE, 200, strFormattedMessage),
            objHandler.Handle(objMessage));

    SetUpPrioritizedReasonHeader(REASON_SIP_PROTOCOL, 603, "\"DECLINED\"");
    strFormattedMessage = "DECLINED";
    EXPECT_EQ(CallReasonInfo(CODE_REJECTED_ELSEWHERE, 603, strFormattedMessage),
            objHandler.Handle(objMessage));
}
TEST_F(CancelHandlerTest, HandleMessageWithExtraMessageFromQ850ReasonHeader)
{
    ON_CALL(*pConfigurationProxy,
            GetBoolean(ConfigVoice::KEY_ENRICH_CALLREASONINFO_WITH_REASON_HEADER_BOOL))
            .WillByDefault(Return(IMS_TRUE));
    SetUpPrioritizedReasonHeader(REASON_Q850_PROTOCOL, 18, "\"No user responding\"");
    AString strFormattedMessage = "q.850;No user responding";
    EXPECT_EQ(CallReasonInfo(CODE_USER_TERMINATED_BY_REMOTE, 18, strFormattedMessage),
            objHandler.Handle(objMessage));
}

TEST_F(CancelHandlerTest, EnrichCallReasonInfoIsSkippedWhenConfigIsFalse)
{
    // Explicitly set the config to false to ensure the enrichment logic is skipped.
    ON_CALL(*pConfigurationProxy,
            GetBoolean(ConfigVoice::KEY_ENRICH_CALLREASONINFO_WITH_REASON_HEADER_BOOL))
            .WillByDefault(Return(IMS_FALSE));
    SetUpPrioritizedReasonHeader(REASON_SIP_PROTOCOL, 200, "\"call completed elsewhere\"");

    // The expected CallReasonInfo only has the code, not the extra message or code.
    EXPECT_EQ(CallReasonInfo(CODE_ANSWERED_ELSEWHERE), objHandler.Handle(objMessage));
}

TEST_F(CancelHandlerTest, GetCodeFromReason_NonSipProtocol_ReturnsTerminatedByRemote)
{
    SetUpPrioritizedReasonHeader(REASON_Q850_PROTOCOL, 200, "any text");

    EXPECT_EQ(CallReasonInfo(CODE_USER_TERMINATED_BY_REMOTE), objHandler.Handle(objMessage));
}

TEST_F(CancelHandlerTest, GetCodeFromReason_SipProtocolBusyText_ReturnsRejectedElsewhere)
{
    SetUpPrioritizedReasonHeader(
            REASON_SIP_PROTOCOL, 999, " another device sent all devices busy response ");

    EXPECT_EQ(CallReasonInfo(CODE_REJECTED_ELSEWHERE), objHandler.Handle(objMessage));
}

TEST_F(CancelHandlerTest, GetCodeFromReason_SipProtocolCompletedText_ReturnsAnsweredElsewhere)
{
    SetUpPrioritizedReasonHeader(REASON_SIP_PROTOCOL, 999, " Call Completion Elsewhere ");

    EXPECT_EQ(CallReasonInfo(CODE_ANSWERED_ELSEWHERE), objHandler.Handle(objMessage));
}

TEST_F(CancelHandlerTest, GetCodeFromReason_SipProtocolCause200_ReturnsAnsweredElsewhere)
{
    SetUpPrioritizedReasonHeader(REASON_SIP_PROTOCOL, 200, "any text");

    EXPECT_EQ(CallReasonInfo(CODE_ANSWERED_ELSEWHERE), objHandler.Handle(objMessage));
}

TEST_F(CancelHandlerTest, GetCodeFromReason_SipProtocolCause600_ReturnsRejectedElsewhere)
{
    SetUpPrioritizedReasonHeader(REASON_SIP_PROTOCOL, 600, "any text");

    EXPECT_EQ(CallReasonInfo(CODE_REJECTED_ELSEWHERE), objHandler.Handle(objMessage));
}

TEST_F(CancelHandlerTest, GetCodeFromReason_SipProtocolCause603_ReturnsRejectedElsewhere)
{
    SetUpPrioritizedReasonHeader(REASON_SIP_PROTOCOL, 603, "any text");

    EXPECT_EQ(CallReasonInfo(CODE_REJECTED_ELSEWHERE), objHandler.Handle(objMessage));
}

TEST_F(CancelHandlerTest, GetCodeFromReason_SipProtocolNoMatch_ReturnsTerminatedByRemote)
{
    SetUpPrioritizedReasonHeader(REASON_SIP_PROTOCOL, 999, "any text");

    EXPECT_EQ(CallReasonInfo(CODE_USER_TERMINATED_BY_REMOTE), objHandler.Handle(objMessage));
}
