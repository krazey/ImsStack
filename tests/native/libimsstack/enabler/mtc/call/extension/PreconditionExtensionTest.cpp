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
 * distributed under the License is distributed on an "AS IS" B ASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "ImsList.h"
#include "ISipHeader.h"
#include "../../../../engine/interface/core/MockIMessage.h"
#include "../../../../engine/interface/sipcore/MockISipMessage.h"
#include "call/extension/MtcExtensionSet.h"
#include "call/extension/PreconditionExtension.h"

using ::testing::_;
using ::testing::Return;

class PreconditionExtensionTest : public ::testing::Test
{
public:
    PreconditionExtension* pExtension;

    MockISipMessage objSipMessageRequiresPrecondition;
    MockISipMessage objSipMessageSupportsPrecondition;
    MockIMessage objMessageRequiresPrecondition;
    MockIMessage objMessageSupportsPrecondition;

protected:
    virtual void SetUp() override
    {
        pExtension = new PreconditionExtension();

        InitMessageRequiresOptionTag(MtcExtensionSet::OPTION_TAG_PRECONDITION);
        InitMessageSupportsOptionTag(MtcExtensionSet::OPTION_TAG_PRECONDITION);
    }

    virtual void TearDown() override
    {
        delete pExtension;
    }

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

    EXPECT_STREQ(
            pExtension->GetOptionTag().GetStr(),
            pCopiedExtension->GetOptionTag().GetStr());
    EXPECT_EQ(
            pExtension->IsAvailableOnRemote(),
            pCopiedExtension->IsAvailableOnRemote());

    delete pCopiedExtension;
}

TEST_F(PreconditionExtensionTest, GetOptionTag)
{
    EXPECT_STREQ(
            MtcExtensionSet::OPTION_TAG_PRECONDITION.GetStr(),
            pExtension->GetOptionTag().GetStr());
}

TEST_F(PreconditionExtensionTest, HandleRequestForSomeMethodDoNothing)
{
    pExtension->HandleRequest(IMessage::SESSION_UPDATE, objMessageRequiresPrecondition);
    EXPECT_FALSE(pExtension->IsAvailableOnRemote());
    EXPECT_FALSE(pExtension->IsRequiredOnRemote());

    pExtension->HandleRequest(IMessage::SESSION_TERMINATE, objMessageRequiresPrecondition);
    EXPECT_FALSE(pExtension->IsAvailableOnRemote());
    EXPECT_FALSE(pExtension->IsRequiredOnRemote());

    pExtension->HandleRequest(IMessage::SESSION_ACK, objMessageRequiresPrecondition);
    EXPECT_FALSE(pExtension->IsAvailableOnRemote());
    EXPECT_FALSE(pExtension->IsRequiredOnRemote());

    pExtension->HandleRequest(IMessage::SESSION_CANCEL, objMessageRequiresPrecondition);
    EXPECT_FALSE(pExtension->IsAvailableOnRemote());
    EXPECT_FALSE(pExtension->IsRequiredOnRemote());
}

TEST_F(PreconditionExtensionTest, HandleRequestForRequiringMessageWithoutSdpDoNothing)
{
    ON_CALL(objSipMessageRequiresPrecondition, GetSdpBodyPart)
            .WillByDefault(Return(nullptr));

    pExtension->HandleRequest(IMessage::SESSION_EARLY_UPDATE, objMessageRequiresPrecondition);
    EXPECT_FALSE(pExtension->IsAvailableOnRemote());
    EXPECT_FALSE(pExtension->IsRequiredOnRemote());

    pExtension->HandleRequest(IMessage::SESSION_PRACK, objMessageRequiresPrecondition);
    EXPECT_FALSE(pExtension->IsAvailableOnRemote());
    EXPECT_FALSE(pExtension->IsRequiredOnRemote());
}

TEST_F(PreconditionExtensionTest, HandleRequestForRequiringMessageWithSdpUpdatesAvailability)
{
    ISipMessageBodyPart* pSdpBody = reinterpret_cast<ISipMessageBodyPart*>(1);
    ON_CALL(objSipMessageRequiresPrecondition, GetSdpBodyPart)
            .WillByDefault(Return(pSdpBody));

    pExtension->HandleRequest(IMessage::SESSION_EARLY_UPDATE, objMessageRequiresPrecondition);
    EXPECT_TRUE(pExtension->IsAvailableOnRemote());
    EXPECT_TRUE(pExtension->IsRequiredOnRemote());

    pExtension->HandleRequest(IMessage::SESSION_PRACK, objMessageRequiresPrecondition);
    EXPECT_TRUE(pExtension->IsAvailableOnRemote());
    EXPECT_TRUE(pExtension->IsRequiredOnRemote());
}

TEST_F(PreconditionExtensionTest, HandleRequestForRequiringStartMessageUpdatesAvailability)
{
    pExtension->HandleRequest(IMessage::SESSION_START, objMessageRequiresPrecondition);
    EXPECT_TRUE(pExtension->IsAvailableOnRemote());
    EXPECT_TRUE(pExtension->IsRequiredOnRemote());
}

TEST_F(PreconditionExtensionTest, HandleRequestForSupportingMessageWithoutSdpDoNothing)
{
    ON_CALL(objSipMessageSupportsPrecondition, GetSdpBodyPart)
            .WillByDefault(Return(nullptr));

    pExtension->HandleRequest(IMessage::SESSION_EARLY_UPDATE, objMessageSupportsPrecondition);
    EXPECT_FALSE(pExtension->IsAvailableOnRemote());
    EXPECT_FALSE(pExtension->IsRequiredOnRemote());

    pExtension->HandleRequest(IMessage::SESSION_PRACK, objMessageSupportsPrecondition);
    EXPECT_FALSE(pExtension->IsAvailableOnRemote());
    EXPECT_FALSE(pExtension->IsRequiredOnRemote());
}

TEST_F(PreconditionExtensionTest, HandleRequestForSupportingMessageWithSdpUpdatesAvailability)
{
    ISipMessageBodyPart* pSdpBody = reinterpret_cast<ISipMessageBodyPart*>(1);
    ON_CALL(objSipMessageSupportsPrecondition, GetSdpBodyPart)
            .WillByDefault(Return(pSdpBody));

    pExtension->HandleRequest(IMessage::SESSION_EARLY_UPDATE, objMessageSupportsPrecondition);
    EXPECT_TRUE(pExtension->IsAvailableOnRemote());
    EXPECT_FALSE(pExtension->IsRequiredOnRemote());

    pExtension->HandleRequest(IMessage::SESSION_PRACK, objMessageSupportsPrecondition);
    EXPECT_TRUE(pExtension->IsAvailableOnRemote());
    EXPECT_FALSE(pExtension->IsRequiredOnRemote());
}

TEST_F(PreconditionExtensionTest, HandleRequestForSupportingStartMessageUpdatesAvailability)
{
    pExtension->HandleRequest(IMessage::SESSION_START, objMessageSupportsPrecondition);
    EXPECT_TRUE(pExtension->IsAvailableOnRemote());
    EXPECT_FALSE(pExtension->IsRequiredOnRemote());
}

TEST_F(PreconditionExtensionTest, HandleResponseForSomeMethodDoNothing)
{
    pExtension->HandleResponse(IMessage::SESSION_UPDATE, objMessageRequiresPrecondition);
    EXPECT_FALSE(pExtension->IsAvailableOnRemote());
    EXPECT_FALSE(pExtension->IsRequiredOnRemote());

    pExtension->HandleResponse(IMessage::SESSION_TERMINATE, objMessageRequiresPrecondition);
    EXPECT_FALSE(pExtension->IsAvailableOnRemote());
    EXPECT_FALSE(pExtension->IsRequiredOnRemote());

    pExtension->HandleResponse(IMessage::SESSION_ACK, objMessageRequiresPrecondition);
    EXPECT_FALSE(pExtension->IsAvailableOnRemote());
    EXPECT_FALSE(pExtension->IsRequiredOnRemote());

    pExtension->HandleResponse(IMessage::SESSION_PRACK, objMessageRequiresPrecondition);
    EXPECT_FALSE(pExtension->IsAvailableOnRemote());
    EXPECT_FALSE(pExtension->IsRequiredOnRemote());

    pExtension->HandleResponse(IMessage::SESSION_EARLY_UPDATE, objMessageRequiresPrecondition);
    EXPECT_FALSE(pExtension->IsAvailableOnRemote());
    EXPECT_FALSE(pExtension->IsRequiredOnRemote());

    pExtension->HandleResponse(IMessage::SESSION_CANCEL, objMessageRequiresPrecondition);
    EXPECT_FALSE(pExtension->IsAvailableOnRemote());
    EXPECT_FALSE(pExtension->IsRequiredOnRemote());
}

TEST_F(PreconditionExtensionTest, HandleResponseForRequiringStartMessageWithoutSdpDoNothing)
{
    ON_CALL(objSipMessageRequiresPrecondition, GetSdpBodyPart)
            .WillByDefault(Return(nullptr));

    pExtension->HandleResponse(IMessage::SESSION_START, objMessageRequiresPrecondition);
    EXPECT_FALSE(pExtension->IsAvailableOnRemote());
    EXPECT_FALSE(pExtension->IsRequiredOnRemote());
}

TEST_F(PreconditionExtensionTest, HandleResponseForRequiringStartMessageWithSdpUpdatesAvailability)
{
    ISipMessageBodyPart* pSdpBody = reinterpret_cast<ISipMessageBodyPart*>(1);
    ON_CALL(objSipMessageRequiresPrecondition, GetSdpBodyPart)
            .WillByDefault(Return(pSdpBody));

    pExtension->HandleResponse(IMessage::SESSION_START, objMessageRequiresPrecondition);
    EXPECT_TRUE(pExtension->IsAvailableOnRemote());
    EXPECT_TRUE(pExtension->IsRequiredOnRemote());
}

TEST_F(PreconditionExtensionTest, HandleResponseForSupportingStartMessageWithoutSdpDoNothing)
{
    ON_CALL(objSipMessageSupportsPrecondition, GetSdpBodyPart)
            .WillByDefault(Return(nullptr));

    pExtension->HandleResponse(IMessage::SESSION_START, objMessageSupportsPrecondition);
    EXPECT_FALSE(pExtension->IsAvailableOnRemote());
    EXPECT_FALSE(pExtension->IsRequiredOnRemote());
}

TEST_F(PreconditionExtensionTest, HandleResponseForSupportingStartMessageWithSdpUpdatesAvailability)
{
    ISipMessageBodyPart* pSdpBody = reinterpret_cast<ISipMessageBodyPart*>(1);
    ON_CALL(objSipMessageSupportsPrecondition, GetSdpBodyPart)
            .WillByDefault(Return(pSdpBody));

    pExtension->HandleResponse(IMessage::SESSION_START, objMessageSupportsPrecondition);
    EXPECT_TRUE(pExtension->IsAvailableOnRemote());
    EXPECT_FALSE(pExtension->IsRequiredOnRemote());
}
