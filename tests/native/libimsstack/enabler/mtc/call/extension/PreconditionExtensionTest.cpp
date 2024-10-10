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
#include "MockISession.h"
#include "MockISipMessage.h"
#include "call/IMtcCall.h"
#include "call/MockIMtcCall.h"
#include "call/MockIMtcCallContext.h"
#include "call/MockIMtcSession.h"
#include "call/extension/MtcExtensionSet.h"
#include "call/extension/PreconditionExtension.h"
#include "media/MockIMtcMediaManager.h"
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
    MockISipMessage objSipMessageRequiresExtension;
    MockISipMessage objSipMessageSupportsExtension;
    MockIMessage objMessage;
    MockIMessage objMessageRequiresExtension;
    MockIMessage objMessageSupportsExtension;
    MockIMtcCallContext objContext;
    MessageUtils objMessageUtils;
    MockIMtcSession objMtcSession;
    MockISession objSession;
    MockIMtcMediaManager objMediaManager;

protected:
    virtual void SetUp() override
    {
        ON_CALL(objContext, GetMessageUtils).WillByDefault(ReturnRef(objMessageUtils));

        pExtension = new PreconditionExtension(objContext);

        ON_CALL(objMessage, GetMessage).WillByDefault(Return(&objSipMessage));

        InitMessageRequiresExtension(MtcExtensionSet::OPTION_TAG_PRECONDITION);
        InitMessageSupportsExtension(MtcExtensionSet::OPTION_TAG_PRECONDITION);
    }

    virtual void TearDown() override { delete pExtension; }

    void InitMessageRequiresExtension(IN const AString& strOptionTag)
    {
        ImsList<AString> lstEmptyHeaders;
        ImsList<AString> lstRequiredHeaders;
        lstRequiredHeaders.Append(strOptionTag);

        ON_CALL(objSipMessageRequiresExtension, GetHeaders(_, _))
                .WillByDefault(Return(lstEmptyHeaders));
        ON_CALL(objSipMessageRequiresExtension, GetHeaders(ISipHeader::REQUIRE, _))
                .WillByDefault(Return(lstRequiredHeaders));

        ON_CALL(objMessageRequiresExtension, GetMessage)
                .WillByDefault(Return(&objSipMessageRequiresExtension));
    }

    void InitMessageSupportsExtension(IN const AString& strOptionTag)
    {
        ImsList<AString> lstEmptyHeaders;
        ImsList<AString> lstSupportedHeaders;
        lstSupportedHeaders.Append(strOptionTag);

        ON_CALL(objSipMessageSupportsExtension, GetHeaders(_, _))
                .WillByDefault(Return(lstEmptyHeaders));
        ON_CALL(objSipMessageSupportsExtension, GetHeaders(ISipHeader::SUPPORTED, _))
                .WillByDefault(Return(lstSupportedHeaders));

        ON_CALL(objMessageSupportsExtension, GetMessage)
                .WillByDefault(Return(&objSipMessageSupportsExtension));
    }

    void SetUpMediaNegoState(IN NegotiationState eNegoState)
    {
        ON_CALL(objContext, GetSession()).WillByDefault(Return(&objMtcSession));
        ON_CALL(objMtcSession, GetISession()).WillByDefault(ReturnRef(objSession));
        ON_CALL(objContext, GetMediaManager).WillByDefault(ReturnRef(objMediaManager));
        ON_CALL(objMediaManager, GetNegotiationState(_)).WillByDefault(Return(eNegoState));
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
    SetUpMediaNegoState(NegotiationState::STATE_OFFER_SENT);
    EXPECT_CALL(objMediaManager, GetNegotiationState(_)).Times(0);

    pExtension->FormatRequest(RequestType::UPDATE, objMessage);
    pExtension->FormatRequest(RequestType::EARLY_UPDATE, objMessage);
    pExtension->FormatRequest(RequestType::PRACK, objMessage);
    pExtension->FormatRequest(RequestType::ACK, objMessage);
    pExtension->FormatRequest(RequestType::CANCEL_UPDATE, objMessage);
    pExtension->FormatRequest(RequestType::TERMINATE, objMessage);
}

TEST_F(PreconditionExtensionTest,
        FormatRequestForSomeMessageDoesNothingIfRemoteAvailableAndNegoStateIsIdle)
{
    // Set remote availability true
    pExtension->HandleRequest(RequestType::START, objMessageRequiresExtension);
    SetUpMediaNegoState(NegotiationState::STATE_IDLE);

    EXPECT_CALL(objMessage, GetMessage).Times(0);

    pExtension->FormatRequest(RequestType::UPDATE, objMessage);
    pExtension->FormatRequest(RequestType::EARLY_UPDATE, objMessage);
    pExtension->FormatRequest(RequestType::PRACK, objMessage);
    pExtension->FormatRequest(RequestType::ACK, objMessage);
    pExtension->FormatRequest(RequestType::CANCEL_UPDATE, objMessage);
    pExtension->FormatRequest(RequestType::TERMINATE, objMessage);
}

TEST_F(PreconditionExtensionTest,
        FormatRequestForSomeMessageDoesNothingIfRemoteAvailableAndNegoStateIsOfferSent)
{
    // Set remote availability true
    pExtension->HandleRequest(RequestType::START, objMessageRequiresExtension);
    SetUpMediaNegoState(NegotiationState::STATE_OFFER_SENT);

    EXPECT_CALL(objMessage, GetMessage).Times(0);

    pExtension->FormatRequest(RequestType::CANCEL_UPDATE, objMessage);
    pExtension->FormatRequest(RequestType::TERMINATE, objMessage);
}

TEST_F(PreconditionExtensionTest, FormatRequestForStartMessageSetsSupportedHeader)
{
    SetUpMediaNegoState(NegotiationState::STATE_OFFER_SENT);
    EXPECT_CALL(objSipMessage, AddHeader(ISipHeader::SUPPORTED, pExtension->GetOptionTag(), _))
            .Times(1);

    pExtension->FormatRequest(RequestType::START, objMessage);
}

TEST_F(PreconditionExtensionTest, FormatRequestForUpdateMessageSetsSupportedHeader)
{
    // Set remote availability true
    pExtension->HandleRequest(RequestType::START, objMessageRequiresExtension);
    SetUpMediaNegoState(NegotiationState::STATE_OFFER_SENT);

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
    pExtension->HandleRequest(RequestType::START, objMessageRequiresExtension);
    SetUpMediaNegoState(NegotiationState::STATE_OFFER_SENT);

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
    pExtension->HandleRequest(RequestType::START, objMessageRequiresExtension);
    SetUpMediaNegoState(NegotiationState::STATE_OFFER_SENT);

    EXPECT_CALL(objSipMessage, AddHeader(ISipHeader::SUPPORTED, pExtension->GetOptionTag(), _))
            .Times(1);
    EXPECT_CALL(objSipMessage, AddHeader(ISipHeader::REQUIRE, _, _)).Times(0);

    pExtension->FormatRequest(RequestType::EARLY_UPDATE, objMessage);
}

TEST_F(PreconditionExtensionTest,
        FormatRequestForPrackMessageSetsRequireHeaderIfRemoteAvailableAndNegoStateIsOfferSent)
{
    // Set remote availability true
    pExtension->HandleRequest(RequestType::START, objMessageRequiresExtension);
    SetUpMediaNegoState(NegotiationState::STATE_OFFER_SENT);

    EXPECT_CALL(objSipMessage, AddHeader(ISipHeader::REQUIRE, pExtension->GetOptionTag(), _))
            .Times(1);
    EXPECT_CALL(objSipMessage, AddHeader(ISipHeader::SUPPORTED, _, _)).Times(0);

    pExtension->FormatRequest(RequestType::PRACK, objMessage);
}

TEST_F(PreconditionExtensionTest,
        FormatRequestForAckMessageSetsSetsRequireHeaderIfRemoteAvailableAndNegoStateIsOfferSent)
{
    // Set remote availability true
    pExtension->HandleRequest(RequestType::START, objMessageRequiresExtension);
    SetUpMediaNegoState(NegotiationState::STATE_OFFER_SENT);

    EXPECT_CALL(objSipMessage, AddHeader(ISipHeader::REQUIRE, pExtension->GetOptionTag(), _))
            .Times(1);
    EXPECT_CALL(objSipMessage, AddHeader(ISipHeader::SUPPORTED, _, _)).Times(0);

    pExtension->FormatRequest(RequestType::ACK, objMessage);
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
    pExtension->HandleRequest(RequestType::START, objMessageRequiresExtension);

    EXPECT_CALL(objSipMessage, AddHeader(_, _, _)).Times(0);

    pExtension->FormatResponse(ResponseType::REJECT, objMessage);
}

TEST_F(PreconditionExtensionTest, FormatResponseSetsRequireHeader)
{
    // Set remote availability true
    pExtension->HandleRequest(RequestType::START, objMessageRequiresExtension);

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
    pExtension->HandleRequest(RequestType::ACK, objMessageRequiresExtension);
    EXPECT_FALSE(pExtension->IsAvailableOnRemote());
    EXPECT_FALSE(pExtension->IsRequiredOnRemote());

    pExtension->HandleRequest(RequestType::UPDATE, objMessageRequiresExtension);
    EXPECT_FALSE(pExtension->IsAvailableOnRemote());
    EXPECT_FALSE(pExtension->IsRequiredOnRemote());

    pExtension->HandleRequest(RequestType::CANCEL_UPDATE, objMessageRequiresExtension);
    EXPECT_FALSE(pExtension->IsAvailableOnRemote());
    EXPECT_FALSE(pExtension->IsRequiredOnRemote());

    pExtension->HandleRequest(RequestType::TERMINATE, objMessageRequiresExtension);
    EXPECT_FALSE(pExtension->IsAvailableOnRemote());
    EXPECT_FALSE(pExtension->IsRequiredOnRemote());
}

TEST_F(PreconditionExtensionTest, HandleRequestForRequiringMessageWithoutSdpDoesNothing)
{
    ON_CALL(objSipMessageRequiresExtension, GetSdpBodyPart).WillByDefault(Return(nullptr));

    pExtension->HandleRequest(RequestType::PRACK, objMessageRequiresExtension);
    EXPECT_FALSE(pExtension->IsAvailableOnRemote());
    EXPECT_FALSE(pExtension->IsRequiredOnRemote());

    pExtension->HandleRequest(RequestType::EARLY_UPDATE, objMessageRequiresExtension);
    EXPECT_FALSE(pExtension->IsAvailableOnRemote());
    EXPECT_FALSE(pExtension->IsRequiredOnRemote());
}

TEST_F(PreconditionExtensionTest, HandleRequestForRequiringMessageWithSdpUpdatesAvailability)
{
    ISipMessageBodyPart* pSdpBody = reinterpret_cast<ISipMessageBodyPart*>(1);
    ON_CALL(objSipMessageRequiresExtension, GetSdpBodyPart).WillByDefault(Return(pSdpBody));

    pExtension->HandleRequest(RequestType::PRACK, objMessageRequiresExtension);
    EXPECT_TRUE(pExtension->IsAvailableOnRemote());
    EXPECT_TRUE(pExtension->IsRequiredOnRemote());

    pExtension->HandleRequest(RequestType::EARLY_UPDATE, objMessageRequiresExtension);
    EXPECT_TRUE(pExtension->IsAvailableOnRemote());
    EXPECT_TRUE(pExtension->IsRequiredOnRemote());
}

TEST_F(PreconditionExtensionTest, HandleRequestForRequiringStartMessageUpdatesAvailability)
{
    pExtension->HandleRequest(RequestType::START, objMessageRequiresExtension);
    EXPECT_TRUE(pExtension->IsAvailableOnRemote());
    EXPECT_TRUE(pExtension->IsRequiredOnRemote());
}

TEST_F(PreconditionExtensionTest, HandleRequestForSupportingMessageWithoutSdpDoesNothing)
{
    ON_CALL(objSipMessageSupportsExtension, GetSdpBodyPart).WillByDefault(Return(nullptr));

    pExtension->HandleRequest(RequestType::PRACK, objMessageSupportsExtension);
    EXPECT_FALSE(pExtension->IsAvailableOnRemote());
    EXPECT_FALSE(pExtension->IsRequiredOnRemote());

    pExtension->HandleRequest(RequestType::EARLY_UPDATE, objMessageSupportsExtension);
    EXPECT_FALSE(pExtension->IsAvailableOnRemote());
    EXPECT_FALSE(pExtension->IsRequiredOnRemote());
}

TEST_F(PreconditionExtensionTest, HandleRequestForSupportingMessageWithSdpUpdatesAvailability)
{
    ISipMessageBodyPart* pSdpBody = reinterpret_cast<ISipMessageBodyPart*>(1);
    ON_CALL(objSipMessageSupportsExtension, GetSdpBodyPart).WillByDefault(Return(pSdpBody));

    pExtension->HandleRequest(RequestType::PRACK, objMessageSupportsExtension);
    EXPECT_TRUE(pExtension->IsAvailableOnRemote());
    EXPECT_FALSE(pExtension->IsRequiredOnRemote());

    pExtension->HandleRequest(RequestType::EARLY_UPDATE, objMessageSupportsExtension);
    EXPECT_TRUE(pExtension->IsAvailableOnRemote());
    EXPECT_FALSE(pExtension->IsRequiredOnRemote());
}

TEST_F(PreconditionExtensionTest, HandleRequestForSupportingStartMessageUpdatesAvailability)
{
    pExtension->HandleRequest(RequestType::START, objMessageSupportsExtension);
    EXPECT_TRUE(pExtension->IsAvailableOnRemote());
    EXPECT_FALSE(pExtension->IsRequiredOnRemote());
}

TEST_F(PreconditionExtensionTest, HandleResponseForSomeMessageDoesNothing)
{
    pExtension->HandleResponse(ResponseType::PRACK_RESPONSE, objMessageRequiresExtension);
    EXPECT_FALSE(pExtension->IsAvailableOnRemote());
    EXPECT_FALSE(pExtension->IsRequiredOnRemote());

    pExtension->HandleResponse(ResponseType::EARLY_UPDATE_RESPONSE, objMessageRequiresExtension);
    EXPECT_FALSE(pExtension->IsAvailableOnRemote());
    EXPECT_FALSE(pExtension->IsRequiredOnRemote());

    pExtension->HandleResponse(ResponseType::ACCEPT, objMessageRequiresExtension);
    EXPECT_FALSE(pExtension->IsAvailableOnRemote());
    EXPECT_FALSE(pExtension->IsRequiredOnRemote());

    pExtension->HandleResponse(ResponseType::REJECT, objMessageRequiresExtension);
    EXPECT_FALSE(pExtension->IsAvailableOnRemote());
    EXPECT_FALSE(pExtension->IsRequiredOnRemote());

    pExtension->HandleResponse(ResponseType::ACCEPT_UPDATE, objMessageRequiresExtension);
    EXPECT_FALSE(pExtension->IsAvailableOnRemote());
    EXPECT_FALSE(pExtension->IsRequiredOnRemote());
}

TEST_F(PreconditionExtensionTest, HandleResponseForRequiringPrMessageWithoutSdpDoesNothing)
{
    ON_CALL(objSipMessageRequiresExtension, GetSdpBodyPart).WillByDefault(Return(nullptr));

    pExtension->HandleResponse(ResponseType::PROVISIONAL_RESPONSE, objMessageRequiresExtension);
    EXPECT_FALSE(pExtension->IsAvailableOnRemote());
    EXPECT_FALSE(pExtension->IsRequiredOnRemote());
}

TEST_F(PreconditionExtensionTest, HandleResponseForRequiringPrMessageWithSdpUpdatesAvailability)
{
    ISipMessageBodyPart* pSdpBody = reinterpret_cast<ISipMessageBodyPart*>(1);
    ON_CALL(objSipMessageRequiresExtension, GetSdpBodyPart).WillByDefault(Return(pSdpBody));

    pExtension->HandleResponse(ResponseType::PROVISIONAL_RESPONSE, objMessageRequiresExtension);
    EXPECT_TRUE(pExtension->IsAvailableOnRemote());
    EXPECT_TRUE(pExtension->IsRequiredOnRemote());
}

TEST_F(PreconditionExtensionTest, HandleResponseForSupportingPrMessageWithoutSdpDoesNothing)
{
    ON_CALL(objSipMessageSupportsExtension, GetSdpBodyPart).WillByDefault(Return(nullptr));

    pExtension->HandleResponse(ResponseType::PROVISIONAL_RESPONSE, objMessageSupportsExtension);
    EXPECT_FALSE(pExtension->IsAvailableOnRemote());
    EXPECT_FALSE(pExtension->IsRequiredOnRemote());
}

TEST_F(PreconditionExtensionTest, HandleResponseForSupportingPrMessageWithSdpUpdatesAvailability)
{
    ISipMessageBodyPart* pSdpBody = reinterpret_cast<ISipMessageBodyPart*>(1);
    ON_CALL(objSipMessageSupportsExtension, GetSdpBodyPart).WillByDefault(Return(pSdpBody));

    pExtension->HandleResponse(ResponseType::PROVISIONAL_RESPONSE, objMessageSupportsExtension);
    EXPECT_TRUE(pExtension->IsAvailableOnRemote());
    EXPECT_FALSE(pExtension->IsRequiredOnRemote());
}
