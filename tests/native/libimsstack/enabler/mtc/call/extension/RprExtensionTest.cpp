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
#include "MockIMtcContext.h"
#include "MtcContextRepository.h"
#include "call/extension/MtcExtensionSet.h"
#include "call/extension/RprExtension.h"
#include "core/MockIMessage.h"
#include "sipcore/MockISipMessage.h"
#include "utility/MessageUtils.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;

class RprExtensionTest : public ::testing::Test
{
public:
    RprExtension* pExtension;

    MockISipMessage objSipMessage;
    MockISipMessage objSipMessageRequiresRpr;
    MockISipMessage objSipMessageSupportsRpr;
    MockIMessage objMessage;
    MockIMessage objMessageRequiresRpr;
    MockIMessage objMessageSupportsRpr;
    MockIMtcContext objContext;
    MessageUtils objMessageUtils;

protected:
    virtual void SetUp() override
    {
        MtcContextRepository::GetInstance()->AddContext(IMS_SLOT_0, &objContext);
        ON_CALL(objContext, GetMessageUtils).WillByDefault(ReturnRef(objMessageUtils));

        pExtension = new RprExtension();

        ON_CALL(objMessage, GetMessage).WillByDefault(Return(&objSipMessage));

        InitMessageRequiresOptionTag(MtcExtensionSet::OPTION_TAG_RPR);
        InitMessageSupportsOptionTag(MtcExtensionSet::OPTION_TAG_RPR);
    }

    virtual void TearDown() override { delete pExtension; }

    void InitMessageRequiresOptionTag(IN const AString& strOptionTag)
    {
        ImsList<AString> lstEmptyHeaders;
        ImsList<AString> lstRequiredHeaders;
        lstRequiredHeaders.Append(strOptionTag);

        ON_CALL(objSipMessageRequiresRpr, GetHeaders(_, _)).WillByDefault(Return(lstEmptyHeaders));
        ON_CALL(objSipMessageRequiresRpr, GetHeaders(ISipHeader::REQUIRE, _))
                .WillByDefault(Return(lstRequiredHeaders));

        ON_CALL(objMessageRequiresRpr, GetMessage).WillByDefault(Return(&objSipMessageRequiresRpr));
    }

    void InitMessageSupportsOptionTag(IN const AString& strOptionTag)
    {
        ImsList<AString> lstEmptyHeaders;
        ImsList<AString> lstSupportedHeaders;
        lstSupportedHeaders.Append(strOptionTag);

        ON_CALL(objSipMessageSupportsRpr, GetHeaders(_, _)).WillByDefault(Return(lstEmptyHeaders));
        ON_CALL(objSipMessageSupportsRpr, GetHeaders(ISipHeader::SUPPORTED, _))
                .WillByDefault(Return(lstSupportedHeaders));

        ON_CALL(objMessageSupportsRpr, GetMessage).WillByDefault(Return(&objSipMessageSupportsRpr));
    }
};

TEST_F(RprExtensionTest, Clone)
{
    IMtcExtension* pCopiedExtension = pExtension->Clone();

    EXPECT_STREQ(pExtension->GetOptionTag().GetStr(), pCopiedExtension->GetOptionTag().GetStr());
    EXPECT_EQ(pExtension->IsAvailableOnRemote(), pCopiedExtension->IsAvailableOnRemote());

    delete pCopiedExtension;
}

TEST_F(RprExtensionTest, GetOptionTag)
{
    EXPECT_STREQ(MtcExtensionSet::OPTION_TAG_RPR.GetStr(), pExtension->GetOptionTag().GetStr());
}

TEST_F(RprExtensionTest, FormatRequestForSomeMessageDoesNothing)
{
    EXPECT_CALL(objSipMessage, AddHeader(_, _, _)).Times(0);

    pExtension->FormatRequest(RequestType::PRACK, objMessage);
    pExtension->FormatRequest(RequestType::EARLY_UPDATE, objMessage);
    pExtension->FormatRequest(RequestType::ACK, objMessage);
    pExtension->FormatRequest(RequestType::CANCEL_UPDATE, objMessage);
    pExtension->FormatRequest(RequestType::TERMINATE, objMessage);
}

TEST_F(RprExtensionTest, FormatRequestForStartMessageSetsSupportedHeader)
{
    EXPECT_CALL(objSipMessage, AddHeader(ISipHeader::SUPPORTED, pExtension->GetOptionTag(), _))
            .Times(1);

    pExtension->FormatRequest(RequestType::START, objMessage);
}

TEST_F(RprExtensionTest, FormatRequestForUpdateMessageSetsSupportedHeader)
{
    EXPECT_CALL(objSipMessage, AddHeader(ISipHeader::SUPPORTED, pExtension->GetOptionTag(), _))
            .Times(1);

    pExtension->FormatRequest(RequestType::UPDATE, objMessage);
}

TEST_F(RprExtensionTest, HandleRequestForSomeMessageDoesNothing)
{
    pExtension->HandleRequest(RequestType::PRACK, objMessageRequiresRpr);
    EXPECT_FALSE(pExtension->IsAvailableOnRemote());
    EXPECT_FALSE(pExtension->IsRequiredOnRemote());

    pExtension->HandleRequest(RequestType::EARLY_UPDATE, objMessageRequiresRpr);
    EXPECT_FALSE(pExtension->IsAvailableOnRemote());
    EXPECT_FALSE(pExtension->IsRequiredOnRemote());

    pExtension->HandleRequest(RequestType::ACK, objMessageRequiresRpr);
    EXPECT_FALSE(pExtension->IsAvailableOnRemote());
    EXPECT_FALSE(pExtension->IsRequiredOnRemote());

    pExtension->HandleRequest(RequestType::UPDATE, objMessageRequiresRpr);
    EXPECT_FALSE(pExtension->IsAvailableOnRemote());
    EXPECT_FALSE(pExtension->IsRequiredOnRemote());

    pExtension->HandleRequest(RequestType::CANCEL_UPDATE, objMessageRequiresRpr);
    EXPECT_FALSE(pExtension->IsAvailableOnRemote());
    EXPECT_FALSE(pExtension->IsRequiredOnRemote());

    pExtension->HandleRequest(RequestType::TERMINATE, objMessageRequiresRpr);
    EXPECT_FALSE(pExtension->IsAvailableOnRemote());
    EXPECT_FALSE(pExtension->IsRequiredOnRemote());
}

TEST_F(RprExtensionTest, HandleRequestForRequiringStartMessageUpdatesAvailability)
{
    pExtension->HandleRequest(RequestType::START, objMessageRequiresRpr);
    EXPECT_TRUE(pExtension->IsAvailableOnRemote());
    EXPECT_TRUE(pExtension->IsRequiredOnRemote());
}

TEST_F(RprExtensionTest, HandleRequestForSupportingStartMessageUpdatesAvailability)
{
    pExtension->HandleRequest(RequestType::START, objMessageSupportsRpr);
    EXPECT_TRUE(pExtension->IsAvailableOnRemote());
    EXPECT_FALSE(pExtension->IsRequiredOnRemote());
}

TEST_F(RprExtensionTest, HandleResponseForSomeMessageDoesNothing)
{
    pExtension->HandleResponse(ResponseType::PRACK_RESPONSE, objMessageRequiresRpr);
    EXPECT_FALSE(pExtension->IsAvailableOnRemote());
    EXPECT_FALSE(pExtension->IsRequiredOnRemote());

    pExtension->HandleResponse(ResponseType::EARLY_UPDATE_RESPONSE, objMessageRequiresRpr);
    EXPECT_FALSE(pExtension->IsAvailableOnRemote());
    EXPECT_FALSE(pExtension->IsRequiredOnRemote());

    pExtension->HandleResponse(ResponseType::ACCEPT, objMessageRequiresRpr);
    EXPECT_FALSE(pExtension->IsAvailableOnRemote());
    EXPECT_FALSE(pExtension->IsRequiredOnRemote());

    pExtension->HandleResponse(ResponseType::REJECT, objMessageRequiresRpr);
    EXPECT_FALSE(pExtension->IsAvailableOnRemote());
    EXPECT_FALSE(pExtension->IsRequiredOnRemote());

    pExtension->HandleResponse(ResponseType::ACCEPT_UPDATE, objMessageRequiresRpr);
    EXPECT_FALSE(pExtension->IsAvailableOnRemote());
    EXPECT_FALSE(pExtension->IsRequiredOnRemote());
}

TEST_F(RprExtensionTest, HandleResponseForRequiringStartMessageUpdatesAvailability)
{
    pExtension->HandleResponse(ResponseType::PROVISIONAL_RESPONSE, objMessageRequiresRpr);
    EXPECT_TRUE(pExtension->IsAvailableOnRemote());
    EXPECT_TRUE(pExtension->IsRequiredOnRemote());
}

TEST_F(RprExtensionTest, HandleResponseForSupportingStartMessageUpdatesAvailability)
{
    pExtension->HandleResponse(ResponseType::PROVISIONAL_RESPONSE, objMessageSupportsRpr);
    EXPECT_TRUE(pExtension->IsAvailableOnRemote());
    EXPECT_FALSE(pExtension->IsRequiredOnRemote());
}
