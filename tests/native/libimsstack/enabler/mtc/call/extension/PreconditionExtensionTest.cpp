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
#include "MockIMessage.h"
#include "MockISession.h"
#include "call/IMtcCall.h"
#include "call/MockIMtcCall.h"
#include "call/MockIMtcCallContext.h"
#include "call/MockIMtcSession.h"
#include "call/extension/MtcExtensionSet.h"
#include "call/extension/PreconditionExtension.h"
#include "media/MockIMtcMediaManager.h"
#include "utility/MockIMessageUtils.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;

class PreconditionExtensionTest : public ::testing::Test
{
public:
    PreconditionExtension* pExtension;

    MockIMessage objMessage;
    MockIMessage objMessageRequiresExtension;
    MockIMessage objMessageSupportsExtension;
    MockIMtcCallContext objContext;
    MockIMessageUtils objMessageUtils;
    MockIMtcSession objMtcSession;
    MockISession objSession;
    MockIMtcMediaManager objMediaManager;

protected:
    virtual void SetUp() override
    {
        ON_CALL(objContext, GetMessageUtils).WillByDefault(ReturnRef(objMessageUtils));
        ON_CALL(objMessageUtils,
                HasValue(&objMessageRequiresExtension, MtcExtensionSet::OPTION_TAG_PRECONDITION,
                        ISipHeader::REQUIRE, _))
                .WillByDefault(Return(IMS_TRUE));
        ON_CALL(objMessageUtils,
                HasValue(&objMessageSupportsExtension, MtcExtensionSet::OPTION_TAG_PRECONDITION,
                        ISipHeader::SUPPORTED, _))
                .WillByDefault(Return(IMS_TRUE));

        pExtension = new PreconditionExtension(objContext);
    }

    virtual void TearDown() override { delete pExtension; }

    void SetUpMediaNegoState(IN NegotiationState eNegoState)
    {
        ON_CALL(objContext, GetSession()).WillByDefault(Return(&objMtcSession));
        ON_CALL(objContext, GetMediaManager).WillByDefault(ReturnRef(objMediaManager));
        ON_CALL(objMtcSession, GetISession()).WillByDefault(ReturnRef(objSession));
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
    EXPECT_CALL(objMessageUtils,
            AddValueIfNotExists(&objMessage, pExtension->GetOptionTag(), ISipHeader::SUPPORTED, _));

    pExtension->FormatRequest(RequestType::START, objMessage);
}

TEST_F(PreconditionExtensionTest, FormatRequestForUpdateMessageSetsSupportedHeader)
{
    // Set remote availability true
    pExtension->HandleRequest(RequestType::START, objMessageRequiresExtension);
    SetUpMediaNegoState(NegotiationState::STATE_OFFER_SENT);

    EXPECT_CALL(objMessageUtils,
            AddValueIfNotExists(&objMessage, pExtension->GetOptionTag(), ISipHeader::SUPPORTED, _));

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

    EXPECT_CALL(objMessageUtils,
            AddValueIfNotExists(&objMessage, pExtension->GetOptionTag(), ISipHeader::SUPPORTED, _))
            .Times(0);
    EXPECT_CALL(objMessageUtils,
            AddValueIfNotExists(&objMessage, pExtension->GetOptionTag(), ISipHeader::REQUIRE, _));

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

    EXPECT_CALL(objMessageUtils,
            AddValueIfNotExists(&objMessage, pExtension->GetOptionTag(), ISipHeader::REQUIRE, _))
            .Times(0);
    EXPECT_CALL(objMessageUtils,
            AddValueIfNotExists(&objMessage, pExtension->GetOptionTag(), ISipHeader::SUPPORTED, _));

    pExtension->FormatRequest(RequestType::EARLY_UPDATE, objMessage);
}

TEST_F(PreconditionExtensionTest,
        FormatRequestForPrackMessageSetsRequireHeaderIfRemoteAvailableAndNegoStateIsOfferSent)
{
    // Set remote availability true
    pExtension->HandleRequest(RequestType::START, objMessageRequiresExtension);
    SetUpMediaNegoState(NegotiationState::STATE_OFFER_SENT);

    EXPECT_CALL(objMessageUtils,
            AddValueIfNotExists(&objMessage, pExtension->GetOptionTag(), ISipHeader::SUPPORTED, _))
            .Times(0);
    EXPECT_CALL(objMessageUtils,
            AddValueIfNotExists(&objMessage, pExtension->GetOptionTag(), ISipHeader::REQUIRE, _));

    pExtension->FormatRequest(RequestType::PRACK, objMessage);
}

TEST_F(PreconditionExtensionTest,
        FormatRequestForAckMessageSetsSetsRequireHeaderIfRemoteAvailableAndNegoStateIsOfferSent)
{
    // Set remote availability true
    pExtension->HandleRequest(RequestType::START, objMessageRequiresExtension);
    SetUpMediaNegoState(NegotiationState::STATE_OFFER_SENT);

    EXPECT_CALL(objMessageUtils,
            AddValueIfNotExists(&objMessage, pExtension->GetOptionTag(), ISipHeader::SUPPORTED, _))
            .Times(0);
    EXPECT_CALL(objMessageUtils,
            AddValueIfNotExists(&objMessage, pExtension->GetOptionTag(), ISipHeader::REQUIRE, _));

    pExtension->FormatRequest(RequestType::ACK, objMessage);
}

TEST_F(PreconditionExtensionTest, FormatResponseDoesNothingIfRemoteNotAvailable)
{
    EXPECT_CALL(objMessageUtils, AddValueIfNotExists(_, _, _, _)).Times(0);

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

    EXPECT_CALL(objMessageUtils, AddValueIfNotExists(_, _, _, _)).Times(0);

    pExtension->FormatResponse(ResponseType::REJECT, objMessage);
}

TEST_F(PreconditionExtensionTest, FormatResponseSetsRequireHeader)
{
    // Set remote availability true
    pExtension->HandleRequest(RequestType::START, objMessageRequiresExtension);

    EXPECT_CALL(objMessageUtils,
            AddValueIfNotExists(&objMessage, pExtension->GetOptionTag(), ISipHeader::REQUIRE, _))
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
    ON_CALL(objMessageUtils, HasSdp(_)).WillByDefault(Return(IMS_FALSE));
    pExtension->HandleRequest(RequestType::PRACK, objMessageRequiresExtension);
    EXPECT_FALSE(pExtension->IsAvailableOnRemote());
    EXPECT_FALSE(pExtension->IsRequiredOnRemote());

    pExtension->HandleRequest(RequestType::EARLY_UPDATE, objMessageRequiresExtension);
    EXPECT_FALSE(pExtension->IsAvailableOnRemote());
    EXPECT_FALSE(pExtension->IsRequiredOnRemote());
}

TEST_F(PreconditionExtensionTest, HandleRequestForRequiringMessageWithSdpUpdatesAvailability)
{
    ON_CALL(objMessageUtils, HasSdp(_)).WillByDefault(Return(IMS_TRUE));

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
    ON_CALL(objMessageUtils, HasSdp(_)).WillByDefault(Return(IMS_FALSE));

    pExtension->HandleRequest(RequestType::PRACK, objMessageSupportsExtension);
    EXPECT_FALSE(pExtension->IsAvailableOnRemote());
    EXPECT_FALSE(pExtension->IsRequiredOnRemote());

    pExtension->HandleRequest(RequestType::EARLY_UPDATE, objMessageSupportsExtension);
    EXPECT_FALSE(pExtension->IsAvailableOnRemote());
    EXPECT_FALSE(pExtension->IsRequiredOnRemote());
}

TEST_F(PreconditionExtensionTest, HandleRequestForSupportingMessageWithSdpUpdatesAvailability)
{
    ON_CALL(objMessageUtils, HasSdp(_)).WillByDefault(Return(IMS_TRUE));

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
    ON_CALL(objMessageUtils, HasSdp(_)).WillByDefault(Return(IMS_FALSE));

    pExtension->HandleResponse(ResponseType::PROVISIONAL_RESPONSE, objMessageRequiresExtension);
    EXPECT_FALSE(pExtension->IsAvailableOnRemote());
    EXPECT_FALSE(pExtension->IsRequiredOnRemote());
}

TEST_F(PreconditionExtensionTest, HandleResponseForRequiringPrMessageWithSdpUpdatesAvailability)
{
    ON_CALL(objMessageUtils, HasSdp(_)).WillByDefault(Return(IMS_TRUE));

    pExtension->HandleResponse(ResponseType::PROVISIONAL_RESPONSE, objMessageRequiresExtension);
    EXPECT_TRUE(pExtension->IsAvailableOnRemote());
    EXPECT_TRUE(pExtension->IsRequiredOnRemote());
}

TEST_F(PreconditionExtensionTest, HandleResponseForSupportingPrMessageWithoutSdpDoesNothing)
{
    ON_CALL(objMessageUtils, HasSdp(_)).WillByDefault(Return(IMS_FALSE));

    pExtension->HandleResponse(ResponseType::PROVISIONAL_RESPONSE, objMessageSupportsExtension);
    EXPECT_FALSE(pExtension->IsAvailableOnRemote());
    EXPECT_FALSE(pExtension->IsRequiredOnRemote());
}

TEST_F(PreconditionExtensionTest, HandleResponseForSupportingPrMessageWithSdpUpdatesAvailability)
{
    ON_CALL(objMessageUtils, HasSdp(_)).WillByDefault(Return(IMS_TRUE));

    pExtension->HandleResponse(ResponseType::PROVISIONAL_RESPONSE, objMessageSupportsExtension);
    EXPECT_TRUE(pExtension->IsAvailableOnRemote());
    EXPECT_FALSE(pExtension->IsRequiredOnRemote());
}
