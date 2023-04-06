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
#include "call/IMtcCall.h"
#include "call/MockIMtcCall.h"
#include "call/MockIMtcCallContext.h"
#include "call/extension/MtcExtensionSet.h"
#include "call/extension/PreconditionExtension.h"
#include "core/MockIMessage.h"
#include "sipcore/MockISipMessage.h"
#include "utility/MessageUtils.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;

class PreconditionExtensionTest : public ::testing::Test
{
public:
    PreconditionExtension* pExtension;

    MockISipMessage objSipMessage;
    MockISipMessage objSipMessageRequiresPrecondition;
    MockISipMessage objSipMessageSupportsPrecondition;
    MockIMessage objMessage;
    MockIMessage objMessageRequiresPrecondition;
    MockIMessage objMessageSupportsPrecondition;
    MockIMtcCallContext objContext;
    MessageUtils objMessageUtils;

protected:
    virtual void SetUp() override
    {
        ON_CALL(objContext, GetMessageUtils).WillByDefault(ReturnRef(objMessageUtils));

        pExtension = new PreconditionExtension(objContext);

        ON_CALL(objMessage, GetMessage).WillByDefault(Return(&objSipMessage));

        InitMessageRequiresOptionTag(MtcExtensionSet::OPTION_TAG_PRECONDITION);
        InitMessageSupportsOptionTag(MtcExtensionSet::OPTION_TAG_PRECONDITION);
    }

    virtual void TearDown() override { delete pExtension; }

    void InitMessageRequiresOptionTag(IN const AString& strOptionTag)
    {
        ImsList<AString> lstEmptyHeaders;
        ImsList<AString> lstRequiredHeaders;
        lstRequiredHeaders.Append(strOptionTag);

        ON_CALL(objSipMessageRequiresPrecondition, GetHeaders(_, _))
                .WillByDefault(Return(lstEmptyHeaders));
        ON_CALL(objSipMessageRequiresPrecondition, GetHeaders(ISipHeader::REQUIRE, _))
                .WillByDefault(Return(lstRequiredHeaders));

        ON_CALL(objMessageRequiresPrecondition, GetMessage)
                .WillByDefault(Return(&objSipMessageRequiresPrecondition));
    }

    void InitMessageSupportsOptionTag(IN const AString& strOptionTag)
    {
        ImsList<AString> lstEmptyHeaders;
        ImsList<AString> lstSupportedHeaders;
        lstSupportedHeaders.Append(strOptionTag);

        ON_CALL(objSipMessageSupportsPrecondition, GetHeaders(_, _))
                .WillByDefault(Return(lstEmptyHeaders));
        ON_CALL(objSipMessageSupportsPrecondition, GetHeaders(ISipHeader::SUPPORTED, _))
                .WillByDefault(Return(lstSupportedHeaders));

        ON_CALL(objMessageSupportsPrecondition, GetMessage)
                .WillByDefault(Return(&objSipMessageSupportsPrecondition));
    }
};

TEST_F(PreconditionExtensionTest, Clone)
{
    IMtcExtension* pCopiedExtension = pExtension->Clone();

    EXPECT_STREQ(pExtension->GetOptionTag().GetStr(), pCopiedExtension->GetOptionTag().GetStr());
    EXPECT_EQ(pExtension->IsAvailableOnRemote(), pCopiedExtension->IsAvailableOnRemote());

    delete pCopiedExtension;
}

TEST_F(PreconditionExtensionTest, GetOptionTag)
{
    EXPECT_STREQ(
            MtcExtensionSet::OPTION_TAG_PRECONDITION.GetStr(), pExtension->GetOptionTag().GetStr());
}

TEST_F(PreconditionExtensionTest, FormatRequestForSomeMessageDoesNothingIfRemoteNotAvailable)
{
    EXPECT_CALL(objSipMessage, AddHeader(_, _, _)).Times(0);

    pExtension->FormatRequest(RequestType::PRACK, objMessage);
    pExtension->FormatRequest(RequestType::EARLY_UPDATE, objMessage);
    pExtension->FormatRequest(RequestType::ACK, objMessage);
    pExtension->FormatRequest(RequestType::UPDATE, objMessage);
    pExtension->FormatRequest(RequestType::CANCEL_UPDATE, objMessage);
    pExtension->FormatRequest(RequestType::TERMINATE, objMessage);
}

TEST_F(PreconditionExtensionTest, FormatRequestForSomeMessageDoesNothingIfRemoteAvailable)
{
    // Set remote availability true
    pExtension->HandleRequest(RequestType::START, objMessageRequiresPrecondition);

    EXPECT_CALL(objSipMessage, AddHeader(_, _, _)).Times(0);

    pExtension->FormatRequest(RequestType::PRACK, objMessage);
    pExtension->FormatRequest(RequestType::ACK, objMessage);
    pExtension->FormatRequest(RequestType::CANCEL_UPDATE, objMessage);
    pExtension->FormatRequest(RequestType::TERMINATE, objMessage);
}

TEST_F(PreconditionExtensionTest, FormatRequestForStartMessageSetsSupportedHeader)
{
    EXPECT_CALL(objSipMessage, AddHeader(ISipHeader::SUPPORTED, pExtension->GetOptionTag(), _))
            .Times(1);

    pExtension->FormatRequest(RequestType::START, objMessage);
}

TEST_F(PreconditionExtensionTest, FormatRequestForUpdateMessageSetsSupportedHeader)
{
    // Set remote availability true
    pExtension->HandleRequest(RequestType::START, objMessageRequiresPrecondition);

    EXPECT_CALL(objSipMessage, AddHeader(ISipHeader::SUPPORTED, pExtension->GetOptionTag(), _))
            .Times(1);

    pExtension->FormatRequest(RequestType::UPDATE, objMessage);
}

TEST_F(PreconditionExtensionTest, FormatRequestForEarlyUpdateMessageSetsRequireHeaderIfOutgoing)
{
    MockIMtcCall objCall;
    ON_CALL(objContext, GetCall).WillByDefault(ReturnRef(objCall));
    ON_CALL(objCall, GetState).WillByDefault(Return(IMtcCall::State::OUTGOING));

    // Set remote availability true
    pExtension->HandleRequest(RequestType::START, objMessageRequiresPrecondition);

    EXPECT_CALL(objSipMessage, AddHeader(ISipHeader::REQUIRE, pExtension->GetOptionTag(), _))
            .Times(1);
    EXPECT_CALL(objSipMessage, AddHeader(ISipHeader::SUPPORTED, _, _)).Times(0);

    pExtension->FormatRequest(RequestType::EARLY_UPDATE, objMessage);
}

TEST_F(PreconditionExtensionTest, FormatRequestForEarlyUpdateMessageSetsSupportedHeaderIfUpdating)
{
    MockIMtcCall objCall;
    ON_CALL(objContext, GetCall).WillByDefault(ReturnRef(objCall));
    ON_CALL(objCall, GetState).WillByDefault(Return(IMtcCall::State::UPDATING));

    // Set remote availability true
    pExtension->HandleRequest(RequestType::START, objMessageRequiresPrecondition);

    EXPECT_CALL(objSipMessage, AddHeader(ISipHeader::SUPPORTED, pExtension->GetOptionTag(), _))
            .Times(1);
    EXPECT_CALL(objSipMessage, AddHeader(ISipHeader::REQUIRE, _, _)).Times(0);

    pExtension->FormatRequest(RequestType::EARLY_UPDATE, objMessage);
}

TEST_F(PreconditionExtensionTest, FormatResponseDoesNothingIfRemoteNotAvailable)
{
    EXPECT_CALL(objSipMessage, AddHeader(_, _, _)).Times(0);

    pExtension->FormatResponse(ResponseType::PROVISIONAL_RESPONSE, objMessage);
    pExtension->FormatResponse(ResponseType::PRACK_RESPONSE, objMessage);
    pExtension->FormatResponse(ResponseType::EARLY_UPDATE_RESPONSE, objMessage);
    pExtension->FormatResponse(ResponseType::ACCEPT, objMessage);
    pExtension->FormatResponse(ResponseType::REJECT, objMessage);
    pExtension->FormatResponse(ResponseType::ACCEPT_UPDATE, objMessage);
}

TEST_F(PreconditionExtensionTest, FormatResponseForRejectMessageDoesNothing)
{
    // Set remote availability true
    pExtension->HandleRequest(RequestType::START, objMessageRequiresPrecondition);

    EXPECT_CALL(objSipMessage, AddHeader(_, _, _)).Times(0);

    pExtension->FormatResponse(ResponseType::REJECT, objMessage);
}

TEST_F(PreconditionExtensionTest, FormatResponseSetsRequireHeader)
{
    // Set remote availability true
    pExtension->HandleRequest(RequestType::START, objMessageRequiresPrecondition);

    EXPECT_CALL(objSipMessage, AddHeader(ISipHeader::REQUIRE, pExtension->GetOptionTag(), _))
            .Times(5);

    pExtension->FormatResponse(ResponseType::PROVISIONAL_RESPONSE, objMessage);
    pExtension->FormatResponse(ResponseType::PRACK_RESPONSE, objMessage);
    pExtension->FormatResponse(ResponseType::EARLY_UPDATE_RESPONSE, objMessage);
    pExtension->FormatResponse(ResponseType::ACCEPT, objMessage);
    pExtension->FormatResponse(ResponseType::ACCEPT_UPDATE, objMessage);
}

TEST_F(PreconditionExtensionTest, HandleRequestForSomeMessageDoesNothing)
{
    pExtension->HandleRequest(RequestType::ACK, objMessageRequiresPrecondition);
    EXPECT_FALSE(pExtension->IsAvailableOnRemote());
    EXPECT_FALSE(pExtension->IsRequiredOnRemote());

    pExtension->HandleRequest(RequestType::UPDATE, objMessageRequiresPrecondition);
    EXPECT_FALSE(pExtension->IsAvailableOnRemote());
    EXPECT_FALSE(pExtension->IsRequiredOnRemote());

    pExtension->HandleRequest(RequestType::CANCEL_UPDATE, objMessageRequiresPrecondition);
    EXPECT_FALSE(pExtension->IsAvailableOnRemote());
    EXPECT_FALSE(pExtension->IsRequiredOnRemote());

    pExtension->HandleRequest(RequestType::TERMINATE, objMessageRequiresPrecondition);
    EXPECT_FALSE(pExtension->IsAvailableOnRemote());
    EXPECT_FALSE(pExtension->IsRequiredOnRemote());
}

TEST_F(PreconditionExtensionTest, HandleRequestForRequiringMessageWithoutSdpDoesNothing)
{
    ON_CALL(objSipMessageRequiresPrecondition, GetSdpBodyPart).WillByDefault(Return(nullptr));

    pExtension->HandleRequest(RequestType::PRACK, objMessageRequiresPrecondition);
    EXPECT_FALSE(pExtension->IsAvailableOnRemote());
    EXPECT_FALSE(pExtension->IsRequiredOnRemote());

    pExtension->HandleRequest(RequestType::EARLY_UPDATE, objMessageRequiresPrecondition);
    EXPECT_FALSE(pExtension->IsAvailableOnRemote());
    EXPECT_FALSE(pExtension->IsRequiredOnRemote());
}

TEST_F(PreconditionExtensionTest, HandleRequestForRequiringMessageWithSdpUpdatesAvailability)
{
    ISipMessageBodyPart* pSdpBody = reinterpret_cast<ISipMessageBodyPart*>(1);
    ON_CALL(objSipMessageRequiresPrecondition, GetSdpBodyPart).WillByDefault(Return(pSdpBody));

    pExtension->HandleRequest(RequestType::PRACK, objMessageRequiresPrecondition);
    EXPECT_TRUE(pExtension->IsAvailableOnRemote());
    EXPECT_TRUE(pExtension->IsRequiredOnRemote());

    pExtension->HandleRequest(RequestType::EARLY_UPDATE, objMessageRequiresPrecondition);
    EXPECT_TRUE(pExtension->IsAvailableOnRemote());
    EXPECT_TRUE(pExtension->IsRequiredOnRemote());
}

TEST_F(PreconditionExtensionTest, HandleRequestForRequiringStartMessageUpdatesAvailability)
{
    pExtension->HandleRequest(RequestType::START, objMessageRequiresPrecondition);
    EXPECT_TRUE(pExtension->IsAvailableOnRemote());
    EXPECT_TRUE(pExtension->IsRequiredOnRemote());
}

TEST_F(PreconditionExtensionTest, HandleRequestForSupportingMessageWithoutSdpDoesNothing)
{
    ON_CALL(objSipMessageSupportsPrecondition, GetSdpBodyPart).WillByDefault(Return(nullptr));

    pExtension->HandleRequest(RequestType::PRACK, objMessageSupportsPrecondition);
    EXPECT_FALSE(pExtension->IsAvailableOnRemote());
    EXPECT_FALSE(pExtension->IsRequiredOnRemote());

    pExtension->HandleRequest(RequestType::EARLY_UPDATE, objMessageSupportsPrecondition);
    EXPECT_FALSE(pExtension->IsAvailableOnRemote());
    EXPECT_FALSE(pExtension->IsRequiredOnRemote());
}

TEST_F(PreconditionExtensionTest, HandleRequestForSupportingMessageWithSdpUpdatesAvailability)
{
    ISipMessageBodyPart* pSdpBody = reinterpret_cast<ISipMessageBodyPart*>(1);
    ON_CALL(objSipMessageSupportsPrecondition, GetSdpBodyPart).WillByDefault(Return(pSdpBody));

    pExtension->HandleRequest(RequestType::PRACK, objMessageSupportsPrecondition);
    EXPECT_TRUE(pExtension->IsAvailableOnRemote());
    EXPECT_FALSE(pExtension->IsRequiredOnRemote());

    pExtension->HandleRequest(RequestType::EARLY_UPDATE, objMessageSupportsPrecondition);
    EXPECT_TRUE(pExtension->IsAvailableOnRemote());
    EXPECT_FALSE(pExtension->IsRequiredOnRemote());
}

TEST_F(PreconditionExtensionTest, HandleRequestForSupportingStartMessageUpdatesAvailability)
{
    pExtension->HandleRequest(RequestType::START, objMessageSupportsPrecondition);
    EXPECT_TRUE(pExtension->IsAvailableOnRemote());
    EXPECT_FALSE(pExtension->IsRequiredOnRemote());
}

TEST_F(PreconditionExtensionTest, HandleResponseForSomeMessageDoesNothing)
{
    pExtension->HandleResponse(ResponseType::PRACK_RESPONSE, objMessageRequiresPrecondition);
    EXPECT_FALSE(pExtension->IsAvailableOnRemote());
    EXPECT_FALSE(pExtension->IsRequiredOnRemote());

    pExtension->HandleResponse(ResponseType::EARLY_UPDATE_RESPONSE, objMessageRequiresPrecondition);
    EXPECT_FALSE(pExtension->IsAvailableOnRemote());
    EXPECT_FALSE(pExtension->IsRequiredOnRemote());

    pExtension->HandleResponse(ResponseType::ACCEPT, objMessageRequiresPrecondition);
    EXPECT_FALSE(pExtension->IsAvailableOnRemote());
    EXPECT_FALSE(pExtension->IsRequiredOnRemote());

    pExtension->HandleResponse(ResponseType::REJECT, objMessageRequiresPrecondition);
    EXPECT_FALSE(pExtension->IsAvailableOnRemote());
    EXPECT_FALSE(pExtension->IsRequiredOnRemote());

    pExtension->HandleResponse(ResponseType::ACCEPT_UPDATE, objMessageRequiresPrecondition);
    EXPECT_FALSE(pExtension->IsAvailableOnRemote());
    EXPECT_FALSE(pExtension->IsRequiredOnRemote());
}

TEST_F(PreconditionExtensionTest, HandleResponseForRequiringStartMessageWithoutSdpDoesNothing)
{
    ON_CALL(objSipMessageRequiresPrecondition, GetSdpBodyPart).WillByDefault(Return(nullptr));

    pExtension->HandleResponse(ResponseType::PROVISIONAL_RESPONSE, objMessageRequiresPrecondition);
    EXPECT_FALSE(pExtension->IsAvailableOnRemote());
    EXPECT_FALSE(pExtension->IsRequiredOnRemote());
}

TEST_F(PreconditionExtensionTest, HandleResponseForRequiringStartMessageWithSdpUpdatesAvailability)
{
    ISipMessageBodyPart* pSdpBody = reinterpret_cast<ISipMessageBodyPart*>(1);
    ON_CALL(objSipMessageRequiresPrecondition, GetSdpBodyPart).WillByDefault(Return(pSdpBody));

    pExtension->HandleResponse(ResponseType::PROVISIONAL_RESPONSE, objMessageRequiresPrecondition);
    EXPECT_TRUE(pExtension->IsAvailableOnRemote());
    EXPECT_TRUE(pExtension->IsRequiredOnRemote());
}

TEST_F(PreconditionExtensionTest, HandleResponseForSupportingStartMessageWithoutSdpDoesNothing)
{
    ON_CALL(objSipMessageSupportsPrecondition, GetSdpBodyPart).WillByDefault(Return(nullptr));

    pExtension->HandleResponse(ResponseType::PROVISIONAL_RESPONSE, objMessageSupportsPrecondition);
    EXPECT_FALSE(pExtension->IsAvailableOnRemote());
    EXPECT_FALSE(pExtension->IsRequiredOnRemote());
}

TEST_F(PreconditionExtensionTest, HandleResponseForSupportingStartMessageWithSdpUpdatesAvailability)
{
    ISipMessageBodyPart* pSdpBody = reinterpret_cast<ISipMessageBodyPart*>(1);
    ON_CALL(objSipMessageSupportsPrecondition, GetSdpBodyPart).WillByDefault(Return(pSdpBody));

    pExtension->HandleResponse(ResponseType::PROVISIONAL_RESPONSE, objMessageSupportsPrecondition);
    EXPECT_TRUE(pExtension->IsAvailableOnRemote());
    EXPECT_FALSE(pExtension->IsRequiredOnRemote());
}
