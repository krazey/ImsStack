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

#include "../../../../engine/interface/core/MockIMessage.h"
#include "../../../../engine/interface/sipcore/MockISipMessage.h"
#include "ISipHeader.h"
#include "ImsList.h"
#include "MockIMtcContext.h"
#include "MtcContextRepository.h"
#include "call/extension/MtcExtension.h"
#include "utility/MessageUtils.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;

const AString strSomeOptionTag = "some_tag";

class MtcExtensionTest : public ::testing::Test
{
public:
    MtcExtension* pExtension;

    MockISipMessage objSipMessageRequiresSomeOptionTag;
    MockISipMessage objSipMessageSupportsSomeOptionTag;
    MockIMessage objMessageRequiresSomeOptionTag;
    MockIMessage objMessageSupportsSomeOptionTag;
    MockIMtcContext objContext;
    MessageUtils objMessageUtils;

protected:
    virtual void SetUp() override
    {
        MtcContextRepository::GetInstance()->AddContext(IMS_SLOT_0, &objContext);
        ON_CALL(objContext, GetMessageUtils).WillByDefault(ReturnRef(objMessageUtils));

        pExtension = new MtcExtension(strSomeOptionTag);

        InitMessageRequiresOptionTag(strSomeOptionTag);
        InitMessageSupportsOptionTag(strSomeOptionTag);
    }

    virtual void TearDown() override { delete pExtension; }

    void InitMessageRequiresOptionTag(IN const AString& strOptionTag)
    {
        ImsList<AString> lstEmptyHeaders;
        ImsList<AString> lstRequiredHeaders;
        lstRequiredHeaders.Append(strOptionTag);

        ON_CALL(objSipMessageRequiresSomeOptionTag, GetHeaders(_, _))
                .WillByDefault(Return(lstEmptyHeaders));
        ON_CALL(objSipMessageRequiresSomeOptionTag, GetHeaders(ISipHeader::REQUIRE, _))
                .WillByDefault(Return(lstRequiredHeaders));

        ON_CALL(objMessageRequiresSomeOptionTag, GetMessage)
                .WillByDefault(Return(&objSipMessageRequiresSomeOptionTag));
    }

    void InitMessageSupportsOptionTag(IN const AString& strOptionTag)
    {
        ImsList<AString> lstEmptyHeaders;
        ImsList<AString> lstSupportedHeaders;
        lstSupportedHeaders.Append(strOptionTag);

        ON_CALL(objSipMessageSupportsSomeOptionTag, GetHeaders(_, _))
                .WillByDefault(Return(lstEmptyHeaders));
        ON_CALL(objSipMessageSupportsSomeOptionTag, GetHeaders(ISipHeader::SUPPORTED, _))
                .WillByDefault(Return(lstSupportedHeaders));

        ON_CALL(objMessageSupportsSomeOptionTag, GetMessage)
                .WillByDefault(Return(&objSipMessageSupportsSomeOptionTag));
    }
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
    EXPECT_STREQ(strSomeOptionTag.GetStr(), pExtension->GetOptionTag().GetStr());
}

TEST_F(MtcExtensionTest, FormatRequestDoesNothing)
{
    pExtension->FormatRequest(RequestType::START, objMessageRequiresSomeOptionTag);
    EXPECT_FALSE(pExtension->IsAvailableOnRemote());

    pExtension->FormatRequest(RequestType::START, objMessageSupportsSomeOptionTag);
    EXPECT_FALSE(pExtension->IsAvailableOnRemote());
}

TEST_F(MtcExtensionTest, FormatResponseDoesNothing)
{
    pExtension->FormatResponse(ResponseType::PROVISIONAL_RESPONSE, objMessageRequiresSomeOptionTag);
    EXPECT_FALSE(pExtension->IsAvailableOnRemote());

    pExtension->FormatResponse(ResponseType::PROVISIONAL_RESPONSE, objMessageSupportsSomeOptionTag);
    EXPECT_FALSE(pExtension->IsAvailableOnRemote());
}

TEST_F(MtcExtensionTest, HandleRequestForRequiringMessageUpdatesAvailability)
{
    pExtension->HandleRequest(RequestType::START, objMessageRequiresSomeOptionTag);
    EXPECT_TRUE(pExtension->IsAvailableOnRemote());
    EXPECT_TRUE(pExtension->IsRequiredOnRemote());
}

TEST_F(MtcExtensionTest, HandleRequestForSupportingMessageUpdatesAvailability)
{
    pExtension->HandleRequest(RequestType::START, objMessageSupportsSomeOptionTag);
    EXPECT_TRUE(pExtension->IsAvailableOnRemote());
    EXPECT_FALSE(pExtension->IsRequiredOnRemote());
}

TEST_F(MtcExtensionTest, HandleResponseForRequiringMessageUpdatesAvailability)
{
    pExtension->HandleResponse(ResponseType::PROVISIONAL_RESPONSE, objMessageRequiresSomeOptionTag);
    EXPECT_TRUE(pExtension->IsAvailableOnRemote());
    EXPECT_TRUE(pExtension->IsRequiredOnRemote());
}

TEST_F(MtcExtensionTest, HandleResponseForSupportingMessageUpdatesAvailability)
{
    pExtension->HandleResponse(ResponseType::PROVISIONAL_RESPONSE, objMessageSupportsSomeOptionTag);
    EXPECT_TRUE(pExtension->IsAvailableOnRemote());
    EXPECT_FALSE(pExtension->IsRequiredOnRemote());
}
