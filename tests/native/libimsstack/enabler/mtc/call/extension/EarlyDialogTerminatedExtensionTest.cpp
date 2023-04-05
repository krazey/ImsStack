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

#include "ISipHeader.h"
#include "call/MockIMtcCallContext.h"
#include "call/extension/EarlyDialogTerminatedExtension.h"
#include "call/extension/MtcExtensionSet.h"
#include "core/MockIMessage.h"
#include "sipcore/MockISipMessage.h"
#include "utility/MockIMessageUtils.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;

class EarlyDialogTerminatedExtensionTest : public ::testing::Test
{
public:
    EarlyDialogTerminatedExtension* pExtension;

    MockIMtcCallContext objContext;
    MockISipMessage objSipMessage;
    MockIMessage objMessage;
    MockIMessageUtils objMessageUtils;

protected:
    virtual void SetUp() override
    {
        ON_CALL(objContext, GetMessageUtils).WillByDefault(ReturnRef(objMessageUtils));
        ON_CALL(objMessage, GetMessage).WillByDefault(Return(&objSipMessage));

        pExtension = new EarlyDialogTerminatedExtension(objContext);
    }

    virtual void TearDown() override { delete pExtension; }
};

TEST_F(EarlyDialogTerminatedExtensionTest, Clone)
{
    IMtcExtension* pCopiedExtension = pExtension->Clone();

    EXPECT_STREQ(pExtension->GetOptionTag().GetStr(), pCopiedExtension->GetOptionTag().GetStr());
    EXPECT_EQ(pExtension->IsAvailableOnRemote(), pCopiedExtension->IsAvailableOnRemote());

    delete pCopiedExtension;
}

TEST_F(EarlyDialogTerminatedExtensionTest, GetOptionTag)
{
    EXPECT_STREQ(MtcExtensionSet::OPTION_TAG_EARLY_DIALOG_TERMINATED.GetStr(),
            pExtension->GetOptionTag().GetStr());
}

TEST_F(EarlyDialogTerminatedExtensionTest, FormatRequestForSomeMessageDoesNothing)
{
    EXPECT_CALL(objSipMessage, AddHeader(_, _, _)).Times(0);

    pExtension->FormatRequest(RequestType::PRACK, objMessage);
    pExtension->FormatRequest(RequestType::EARLY_UPDATE, objMessage);
    pExtension->FormatRequest(RequestType::ACK, objMessage);
    pExtension->FormatRequest(RequestType::UPDATE, objMessage);
    pExtension->FormatRequest(RequestType::CANCEL_UPDATE, objMessage);
    pExtension->FormatRequest(RequestType::TERMINATE, objMessage);
}

TEST_F(EarlyDialogTerminatedExtensionTest, FormatRequestAddsSupportedHeader)
{
    EXPECT_CALL(objSipMessage, AddHeader(ISipHeader::SUPPORTED, pExtension->GetOptionTag(), _))
            .Times(0);

    pExtension->HandleRequest(RequestType::START, objMessage);
}
