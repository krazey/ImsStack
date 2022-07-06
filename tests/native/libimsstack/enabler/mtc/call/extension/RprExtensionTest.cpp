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

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "ImsList.h"
#include "ISipHeader.h"
#include "../../../../engine/interface/core/MockIMessage.h"
#include "../../../../engine/interface/sipcore/MockISipMessage.h"
#include "call/extension/MtcExtensionSet.h"
#include "call/extension/RprExtension.h"

using ::testing::_;
using ::testing::Return;

class RprExtensionTest : public ::testing::Test
{
public:
    RprExtension* pExtension;

    MockISipMessage objSipMessageRequiresRpr;
    MockISipMessage objSipMessageSupportsRpr;
    MockIMessage objMessageRequiresRpr;
    MockIMessage objMessageSupportsRpr;

protected:
    virtual void SetUp() override
    {
        pExtension = new RprExtension();

        InitMessageRequiresOptionTag(MtcExtensionSet::OPTION_TAG_RPR);
        InitMessageSupportsOptionTag(MtcExtensionSet::OPTION_TAG_RPR);
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

        ON_CALL(objSipMessageRequiresRpr, GetHeaders(_, _))
                .WillByDefault(Return(lstEmptyHeaders));
        ON_CALL(objSipMessageRequiresRpr, GetHeaders(ISipHeader::REQUIRE, _))
                .WillByDefault(Return(lstRequiredHeaders));

        ON_CALL(objMessageRequiresRpr, GetMessage)
                .WillByDefault(Return(&objSipMessageRequiresRpr));
    }

    void InitMessageSupportsOptionTag(IN const AString& strOptionTag)
    {
        ImsList<AString> lstEmptyHeaders;
        ImsList<AString> lstSupportedHeaders;
        lstSupportedHeaders.Append(strOptionTag);

        ON_CALL(objSipMessageSupportsRpr, GetHeaders(_, _))
                .WillByDefault(Return(lstEmptyHeaders));
        ON_CALL(objSipMessageSupportsRpr, GetHeaders(ISipHeader::SUPPORTED, _))
                .WillByDefault(Return(lstSupportedHeaders));

        ON_CALL(objMessageSupportsRpr, GetMessage)
                .WillByDefault(Return(&objSipMessageSupportsRpr));
    }
};

TEST_F(RprExtensionTest, Clone)
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

TEST_F(RprExtensionTest, GetOptionTag)
{
    EXPECT_STREQ(
            MtcExtensionSet::OPTION_TAG_RPR.GetStr(),
            pExtension->GetOptionTag().GetStr());
}

TEST_F(RprExtensionTest, HandleRequestForSomeMethodDoNothing)
{
    pExtension->HandleRequest(IMessage::SESSION_UPDATE, objMessageRequiresRpr);
    EXPECT_FALSE(pExtension->IsAvailableOnRemote());
    EXPECT_FALSE(pExtension->IsRequiredOnRemote());

    pExtension->HandleRequest(IMessage::SESSION_TERMINATE, objMessageRequiresRpr);
    EXPECT_FALSE(pExtension->IsAvailableOnRemote());
    EXPECT_FALSE(pExtension->IsRequiredOnRemote());

    pExtension->HandleRequest(IMessage::SESSION_ACK, objMessageRequiresRpr);
    EXPECT_FALSE(pExtension->IsAvailableOnRemote());
    EXPECT_FALSE(pExtension->IsRequiredOnRemote());

    pExtension->HandleRequest(IMessage::SESSION_PRACK, objMessageRequiresRpr);
    EXPECT_FALSE(pExtension->IsAvailableOnRemote());
    EXPECT_FALSE(pExtension->IsRequiredOnRemote());

    pExtension->HandleRequest(IMessage::SESSION_EARLY_UPDATE, objMessageRequiresRpr);
    EXPECT_FALSE(pExtension->IsAvailableOnRemote());
    EXPECT_FALSE(pExtension->IsRequiredOnRemote());

    pExtension->HandleRequest(IMessage::SESSION_CANCEL, objMessageRequiresRpr);
    EXPECT_FALSE(pExtension->IsAvailableOnRemote());
    EXPECT_FALSE(pExtension->IsRequiredOnRemote());
}

TEST_F(RprExtensionTest, HandleRequestForRequiringStartMessageUpdatesAvailability)
{
    pExtension->HandleRequest(IMessage::SESSION_START, objMessageRequiresRpr);
    EXPECT_TRUE(pExtension->IsAvailableOnRemote());
    EXPECT_TRUE(pExtension->IsRequiredOnRemote());
}

TEST_F(RprExtensionTest, HandleRequestForSupportingStartMessageUpdatesAvailability)
{
    pExtension->HandleRequest(IMessage::SESSION_START, objMessageSupportsRpr);
    EXPECT_TRUE(pExtension->IsAvailableOnRemote());
    EXPECT_FALSE(pExtension->IsRequiredOnRemote());
}

TEST_F(RprExtensionTest, HandleResponseForSomeMethodDoNothing)
{
    pExtension->HandleResponse(IMessage::SESSION_UPDATE, objMessageRequiresRpr);
    EXPECT_FALSE(pExtension->IsAvailableOnRemote());
    EXPECT_FALSE(pExtension->IsRequiredOnRemote());

    pExtension->HandleResponse(IMessage::SESSION_TERMINATE, objMessageRequiresRpr);
    EXPECT_FALSE(pExtension->IsAvailableOnRemote());
    EXPECT_FALSE(pExtension->IsRequiredOnRemote());

    pExtension->HandleResponse(IMessage::SESSION_ACK, objMessageRequiresRpr);
    EXPECT_FALSE(pExtension->IsAvailableOnRemote());
    EXPECT_FALSE(pExtension->IsRequiredOnRemote());

    pExtension->HandleResponse(IMessage::SESSION_PRACK, objMessageRequiresRpr);
    EXPECT_FALSE(pExtension->IsAvailableOnRemote());
    EXPECT_FALSE(pExtension->IsRequiredOnRemote());

    pExtension->HandleResponse(IMessage::SESSION_EARLY_UPDATE, objMessageRequiresRpr);
    EXPECT_FALSE(pExtension->IsAvailableOnRemote());
    EXPECT_FALSE(pExtension->IsRequiredOnRemote());

    pExtension->HandleResponse(IMessage::SESSION_CANCEL, objMessageRequiresRpr);
    EXPECT_FALSE(pExtension->IsAvailableOnRemote());
    EXPECT_FALSE(pExtension->IsRequiredOnRemote());
}

TEST_F(RprExtensionTest, HandleResponseForRequiringStartMessageUpdatesAvailability)
{
    pExtension->HandleResponse(IMessage::SESSION_START, objMessageRequiresRpr);
    EXPECT_TRUE(pExtension->IsAvailableOnRemote());
    EXPECT_TRUE(pExtension->IsRequiredOnRemote());
}

TEST_F(RprExtensionTest, HandleResponseForSupportingStartMessageUpdatesAvailability)
{
    pExtension->HandleResponse(IMessage::SESSION_START, objMessageSupportsRpr);
    EXPECT_TRUE(pExtension->IsAvailableOnRemote());
    EXPECT_FALSE(pExtension->IsRequiredOnRemote());
}
