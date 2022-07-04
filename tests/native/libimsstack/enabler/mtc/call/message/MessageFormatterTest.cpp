/*
 * Copyright (C) 2021 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" B ASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <gtest/gtest.h>
#include "call/message/MessageFormatter.h"
#include "call/MockIMtcSessionContext.h"
#include "CallReasonInfo.h"
#include "sipcore/SipStatusCode.h"
#include "configuration/MockIMtcConfigurationManager.h"
#include "configuration/MtcConfigurationProxy.h"
#include "../../../engine/interface/core/MockISession.h"
#include "../../../engine/interface/core/MockIMessage.h"

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;

namespace android
{

class MessageFormatterTest : public ::testing::Test
{
public:
    MessageFormatter* pFormatter;

    MockIMessage objNextRequestMessage;
    MockIMessage objNextResponseMessage;
    MockISession objSession;
    MockIMtcSessionContext objContext;
    MockIMtcConfigurationManager objMockConfigurationManager;
    MtcConfigurationProxy* pConfigurationProxy;

protected:
    virtual void SetUp() override
    {
        ON_CALL(objSession, GetNextRequest).WillByDefault(Return(&objNextRequestMessage));
        ON_CALL(objSession, GetNextResponse).WillByDefault(Return(&objNextResponseMessage));

        ON_CALL(objContext, GetISession()).WillByDefault(ReturnRef(objSession));

        pFormatter = new MessageFormatter(objContext);
    }

    virtual void TearDown() override { delete pFormatter; }

    void CreateConfiguration()
    {
        pConfigurationProxy = new MtcConfigurationProxy(&objMockConfigurationManager);
        ON_CALL(objContext, GetConfigurationProxy).WillByDefault(ReturnRef(*pConfigurationProxy));
    }
};

TEST_F(MessageFormatterTest, FormRejectWithCodeMediaNotAcceptable)
{
    CreateConfiguration();
    const AString REJECT_PHRASE = "TEST_PHRASE";
    ON_CALL(objMockConfigurationManager, GetCallRejectReasonPhrase(_))
            .WillByDefault(Return(REJECT_PHRASE));

    CallReasonInfo objReason(CODE_MEDIA_NOT_ACCEPTABLE);
    IMS_SINT32 nStatusCode;
    AString strPhrase;
    pFormatter->FormRejectMessage(objReason, nStatusCode, strPhrase);

    EXPECT_EQ(nStatusCode, SipStatusCode::SC_488);
    EXPECT_EQ(strPhrase, REJECT_PHRASE);
}

TEST_F(MessageFormatterTest, FormRejectWithCodeMediaInitFailed)
{
    CreateConfiguration();
    const AString REJECT_PHRASE = "TEST_PHRASE";
    ON_CALL(objMockConfigurationManager, GetCallRejectReasonPhrase(_))
            .WillByDefault(Return(REJECT_PHRASE));

    CallReasonInfo objReason(CODE_MEDIA_INIT_FAILED);
    IMS_SINT32 nStatusCode;
    AString strPhrase;
    pFormatter->FormRejectMessage(objReason, nStatusCode, strPhrase);

    EXPECT_EQ(nStatusCode, SipStatusCode::SC_480);
    EXPECT_EQ(strPhrase, "");
}

}  // namespace android
