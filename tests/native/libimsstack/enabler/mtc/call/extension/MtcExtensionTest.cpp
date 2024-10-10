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
#include "ImsList.h"
#include "MockIMessage.h"
#include "call/MockIMtcCallContext.h"
#include "call/extension/MtcExtension.h"
#include "utility/MockIMessageUtils.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;

const LOCAL AString OPTION_TAG = "some_tag";

class MtcExtensionTest : public ::testing::Test
{
public:
    MtcExtension* pExtension;

    MockIMessage objMessage;
    MockIMtcCallContext objContext;
    MockIMessageUtils objMessageUtils;

protected:
    virtual void SetUp() override
    {
        ON_CALL(objContext, GetMessageUtils).WillByDefault(ReturnRef(objMessageUtils));

        pExtension = new MtcExtension(
                objContext, OPTION_TAG, {RequestType::START}, {ResponseType::PROVISIONAL_RESPONSE});
    }

    virtual void TearDown() override { delete pExtension; }
};

TEST_F(MtcExtensionTest, CopyConstructor)
{
    MtcExtension objCopiedExtension(*pExtension);

    EXPECT_STREQ(pExtension->GetOptionTag().GetStr(), objCopiedExtension.GetOptionTag().GetStr());
    EXPECT_EQ(pExtension->IsAvailableOnRemote(), objCopiedExtension.IsAvailableOnRemote());
}

TEST_F(MtcExtensionTest, Clone)
{
    IMtcExtension* pCopiedExtension = pExtension->Clone();

    EXPECT_STREQ(pExtension->GetOptionTag().GetStr(), pCopiedExtension->GetOptionTag().GetStr());
    EXPECT_EQ(pExtension->IsAvailableOnRemote(), pCopiedExtension->IsAvailableOnRemote());

    delete pCopiedExtension;
}

TEST_F(MtcExtensionTest, IsAvailableOnRemoteReturnsFalseInitially)
{
    EXPECT_FALSE(pExtension->IsAvailableOnRemote());
}

TEST_F(MtcExtensionTest, IsRequiredOnRemoteReturnsFalseInitially)
{
    EXPECT_FALSE(pExtension->IsRequiredOnRemote());
}

TEST_F(MtcExtensionTest, GetOptionTag)
{
    EXPECT_STREQ(OPTION_TAG.GetStr(), pExtension->GetOptionTag().GetStr());
}

TEST_F(MtcExtensionTest, FormatRequestDoesNothingIfNotSupportedMessage)
{
    pExtension->FormatRequest(RequestType::TERMINATE, objMessage);
    EXPECT_FALSE(pExtension->IsAvailableOnRemote());

    pExtension->FormatRequest(RequestType::TERMINATE, objMessage);
    EXPECT_FALSE(pExtension->IsAvailableOnRemote());
}

TEST_F(MtcExtensionTest, FormatRequestDoesNothingIfNotAvailableOnRemote)
{
    pExtension->FormatRequest(RequestType::START, objMessage);
    EXPECT_FALSE(pExtension->IsAvailableOnRemote());

    pExtension->FormatRequest(RequestType::START, objMessage);
    EXPECT_FALSE(pExtension->IsAvailableOnRemote());
}

TEST_F(MtcExtensionTest, FormatRequestAddsOptionTagIfSupportedOnRemote)
{
    ON_CALL(objMessageUtils, HasValue(_, OPTION_TAG, ISipHeader::SUPPORTED, _))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objMessageUtils, HasValue(_, OPTION_TAG, ISipHeader::REQUIRE, _))
            .WillByDefault(Return(IMS_FALSE));
    pExtension->HandleResponse(ResponseType::PROVISIONAL_RESPONSE, objMessage);

    EXPECT_CALL(objMessageUtils,
            AddValueIfNotExists(&objMessage, OPTION_TAG, ISipHeader::SUPPORTED, _));
    pExtension->FormatRequest(RequestType::START, objMessage);
}

TEST_F(MtcExtensionTest, FormatRequestAddsOptionTagIfRequiredOnRemote)
{
    ON_CALL(objMessageUtils, HasValue(_, OPTION_TAG, ISipHeader::SUPPORTED, _))
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMessageUtils, HasValue(_, OPTION_TAG, ISipHeader::REQUIRE, _))
            .WillByDefault(Return(IMS_TRUE));
    pExtension->HandleResponse(ResponseType::PROVISIONAL_RESPONSE, objMessage);

    EXPECT_CALL(objMessageUtils,
            AddValueIfNotExists(&objMessage, OPTION_TAG, ISipHeader::SUPPORTED, _));
    pExtension->FormatRequest(RequestType::START, objMessage);
}

TEST_F(MtcExtensionTest, FormatResponseDoesNothingIfNotSupportedMessage)
{
    pExtension->FormatResponse(ResponseType::REJECT, objMessage);
    EXPECT_FALSE(pExtension->IsAvailableOnRemote());

    pExtension->FormatResponse(ResponseType::REJECT, objMessage);
    EXPECT_FALSE(pExtension->IsAvailableOnRemote());
}

TEST_F(MtcExtensionTest, FormatResponseDoesNothingIfNotAvailableOnRemote)
{
    pExtension->FormatResponse(ResponseType::PROVISIONAL_RESPONSE, objMessage);
    EXPECT_FALSE(pExtension->IsAvailableOnRemote());

    pExtension->FormatResponse(ResponseType::PROVISIONAL_RESPONSE, objMessage);
    EXPECT_FALSE(pExtension->IsAvailableOnRemote());
}

TEST_F(MtcExtensionTest, FormatResponseAddOptionTagIfSupportedOnRemote)
{
    ON_CALL(objMessageUtils, HasValue(_, OPTION_TAG, ISipHeader::SUPPORTED, _))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objMessageUtils, HasValue(_, OPTION_TAG, ISipHeader::REQUIRE, _))
            .WillByDefault(Return(IMS_FALSE));
    pExtension->HandleResponse(ResponseType::PROVISIONAL_RESPONSE, objMessage);

    MockIMessage objMessage;
    EXPECT_CALL(objMessageUtils,
            AddValueIfNotExists(&objMessage, OPTION_TAG, ISipHeader::SUPPORTED, _));
    pExtension->FormatResponse(ResponseType::PROVISIONAL_RESPONSE, objMessage);
}

TEST_F(MtcExtensionTest, FormatResponseAddOptionTagIfRequiredOnRemote)
{
    ON_CALL(objMessageUtils, HasValue(_, OPTION_TAG, ISipHeader::SUPPORTED, _))
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMessageUtils, HasValue(_, OPTION_TAG, ISipHeader::REQUIRE, _))
            .WillByDefault(Return(IMS_TRUE));
    pExtension->HandleResponse(ResponseType::PROVISIONAL_RESPONSE, objMessage);

    MockIMessage objMessage;
    EXPECT_CALL(objMessageUtils,
            AddValueIfNotExists(&objMessage, OPTION_TAG, ISipHeader::SUPPORTED, _));
    pExtension->FormatResponse(ResponseType::PROVISIONAL_RESPONSE, objMessage);
}

TEST_F(MtcExtensionTest, HandleRequestForSupportedRequiringMessageUpdatesAvailability)
{
    ON_CALL(objMessageUtils, HasValue(_, OPTION_TAG, ISipHeader::SUPPORTED, _))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objMessageUtils, HasValue(_, OPTION_TAG, ISipHeader::REQUIRE, _))
            .WillByDefault(Return(IMS_TRUE));

    pExtension->HandleRequest(RequestType::START, objMessage);
    EXPECT_TRUE(pExtension->IsAvailableOnRemote());
    EXPECT_TRUE(pExtension->IsRequiredOnRemote());
}

TEST_F(MtcExtensionTest, HandleRequestForSupportedSupportingMessageUpdatesAvailability)
{
    ON_CALL(objMessageUtils, HasValue(_, OPTION_TAG, ISipHeader::SUPPORTED, _))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objMessageUtils, HasValue(_, OPTION_TAG, ISipHeader::REQUIRE, _))
            .WillByDefault(Return(IMS_FALSE));

    pExtension->HandleRequest(RequestType::START, objMessage);
    EXPECT_TRUE(pExtension->IsAvailableOnRemote());
    EXPECT_FALSE(pExtension->IsRequiredOnRemote());
}

TEST_F(MtcExtensionTest, HandleResponseForSupportedRequiringMessageUpdatesAvailability)
{
    ON_CALL(objMessageUtils, HasValue(_, OPTION_TAG, ISipHeader::SUPPORTED, _))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objMessageUtils, HasValue(_, OPTION_TAG, ISipHeader::REQUIRE, _))
            .WillByDefault(Return(IMS_TRUE));

    pExtension->HandleResponse(ResponseType::PROVISIONAL_RESPONSE, objMessage);
    EXPECT_TRUE(pExtension->IsAvailableOnRemote());
    EXPECT_TRUE(pExtension->IsRequiredOnRemote());
}

TEST_F(MtcExtensionTest, HandleResponseForSupportedSupportingMessageUpdatesAvailability)
{
    ON_CALL(objMessageUtils, HasValue(_, OPTION_TAG, ISipHeader::SUPPORTED, _))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objMessageUtils, HasValue(_, OPTION_TAG, ISipHeader::REQUIRE, _))
            .WillByDefault(Return(IMS_FALSE));

    pExtension->HandleResponse(ResponseType::PROVISIONAL_RESPONSE, objMessage);
    EXPECT_TRUE(pExtension->IsAvailableOnRemote());
    EXPECT_FALSE(pExtension->IsRequiredOnRemote());
}

TEST_F(MtcExtensionTest, HandleRequestForNotSupportedMessageUpdatesAvailability)
{
    ON_CALL(objMessageUtils, HasValue(_, OPTION_TAG, ISipHeader::SUPPORTED, _))
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMessageUtils, HasValue(_, OPTION_TAG, ISipHeader::REQUIRE, _))
            .WillByDefault(Return(IMS_FALSE));

    pExtension->HandleRequest(RequestType::TERMINATE, objMessage);
    pExtension->HandleRequest(RequestType::TERMINATE, objMessage);
    EXPECT_FALSE(pExtension->IsAvailableOnRemote());
    EXPECT_FALSE(pExtension->IsRequiredOnRemote());
}

TEST_F(MtcExtensionTest, HandleResponseForNotSupportedMessageUpdatesAvailability)
{
    ON_CALL(objMessageUtils, HasValue(_, OPTION_TAG, ISipHeader::SUPPORTED, _))
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMessageUtils, HasValue(_, OPTION_TAG, ISipHeader::REQUIRE, _))
            .WillByDefault(Return(IMS_FALSE));

    pExtension->HandleResponse(ResponseType::REJECT, objMessage);
    pExtension->HandleResponse(ResponseType::REJECT, objMessage);
    EXPECT_FALSE(pExtension->IsAvailableOnRemote());
    EXPECT_FALSE(pExtension->IsRequiredOnRemote());
}
