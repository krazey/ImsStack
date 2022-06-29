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
#include "IMessage.h"
#include "ImsList.h"
#include "ISipMessage.h"
#include "../../../../engine/interface/core/MockIMessage.h"
#include "../../../../engine/interface/sipcore/MockISipMessage.h"
#include "call/extension/IMtcExtension.h"
#include "call/extension/MockIMtcExtension.h"
#include "call/extension/MtcExtension.h"
#include "call/extension/MtcExtensionSet.h"
#include "call/extension/RprExtension.h"

using ::testing::_;
using ::testing::Eq;
using ::testing::Ref;
using ::testing::Return;
using ::testing::ReturnRef;

MtcExtensionSet CreateExtensionSetSupportsRprOnly()
{
    ImsList<IMtcExtension*> lstExtensions;
    lstExtensions.Append(new RprExtension());

    return MtcExtensionSet(lstExtensions);
}

MtcExtensionSet CreateExtensionSetSupportsTimerOnly()
{
    ImsList<IMtcExtension*> lstExtensions;
    lstExtensions.Append(new MtcExtension(MtcExtensionSet::OPTION_TAG_TIMER));

    return MtcExtensionSet(lstExtensions);
}

TEST(MtcExtensionSetTest, ConstructorWithEmptyOptionTags)
{
    ImsList<IMtcExtension*> lstEmptyExtensions;
    MtcExtensionSet objExtensionSet(lstEmptyExtensions);

    EXPECT_FALSE(objExtensionSet.IsAvailableOnLocal(
            MtcExtensionSet::OPTION_TAG_EARLY_DIALOG_TERMINATED));
    EXPECT_FALSE(objExtensionSet.IsAvailableOnLocal(MtcExtensionSet::OPTION_TAG_FROM_CHANGE));
    EXPECT_FALSE(objExtensionSet.IsAvailableOnLocal(MtcExtensionSet::OPTION_TAG_HISTORY_INFO));
    EXPECT_FALSE(objExtensionSet.IsAvailableOnLocal(MtcExtensionSet::OPTION_TAG_PRECONDITION));
    EXPECT_FALSE(objExtensionSet.IsAvailableOnLocal(MtcExtensionSet::OPTION_TAG_REPLACES));
    EXPECT_FALSE(objExtensionSet.IsAvailableOnLocal(MtcExtensionSet::OPTION_TAG_RPR));
    EXPECT_FALSE(objExtensionSet.IsAvailableOnLocal(MtcExtensionSet::OPTION_TAG_TARGET_DIALOG));
    EXPECT_FALSE(objExtensionSet.IsAvailableOnLocal(MtcExtensionSet::OPTION_TAG_TIMER));
}

TEST(MtcExtensionSetTest, CopyConstructor)
{
    MtcExtensionSet objExtensionSet = CreateExtensionSetSupportsRprOnly();
    MtcExtensionSet objCopiedExtensionSet(objExtensionSet);

    EXPECT_EQ(
            objExtensionSet.IsAvailableOnLocal(MtcExtensionSet::OPTION_TAG_RPR),
            objCopiedExtensionSet.IsAvailableOnLocal(MtcExtensionSet::OPTION_TAG_RPR));
    EXPECT_EQ(
            objExtensionSet.IsAvailableOnLocal(MtcExtensionSet::OPTION_TAG_TIMER),
            objCopiedExtensionSet.IsAvailableOnLocal(MtcExtensionSet::OPTION_TAG_TIMER));
}

TEST(MtcExtensionSetTest, AssignOperator)
{
    MtcExtensionSet objAssignedExtensionSet = CreateExtensionSetSupportsTimerOnly();
    MtcExtensionSet objExtensionSet = CreateExtensionSetSupportsRprOnly();

    objAssignedExtensionSet = objExtensionSet;

    EXPECT_TRUE(objAssignedExtensionSet.IsAvailableOnLocal(MtcExtensionSet::OPTION_TAG_RPR));
    EXPECT_FALSE(objAssignedExtensionSet.IsAvailableOnLocal(MtcExtensionSet::OPTION_TAG_TIMER));
}

TEST(MtcExtensionSetTest, IsAvailableOnBothInitiallyReturnsFalse)
{
    MtcExtensionSet objExtensionSet = CreateExtensionSetSupportsRprOnly();

    EXPECT_FALSE(objExtensionSet.IsAvailableOnBoth(MtcExtensionSet::OPTION_TAG_RPR));
    EXPECT_FALSE(objExtensionSet.IsAvailableOnBoth(MtcExtensionSet::OPTION_TAG_TIMER));
}

TEST(MtcExtensionSetTest, IsAvailableOnLocalInitiallyReturnsInitialValue)
{
    MtcExtensionSet objExtensionSet = CreateExtensionSetSupportsRprOnly();

    EXPECT_TRUE(objExtensionSet.IsAvailableOnLocal(MtcExtensionSet::OPTION_TAG_RPR));
    EXPECT_FALSE(objExtensionSet.IsAvailableOnLocal(MtcExtensionSet::OPTION_TAG_TIMER));
}

TEST(MtcExtensionSetTest, IsSupportRequiredExtensionsReturnsTrueForNotAvailableExtension)
{
    MtcExtensionSet objExtensionSet = CreateExtensionSetSupportsRprOnly();

    ImsList<AString> lstRequiredHeaders;
    lstRequiredHeaders.Append(MtcExtensionSet::OPTION_TAG_RPR);

    MockISipMessage objSipMessageRequiresRpr;
    ON_CALL(objSipMessageRequiresRpr, GetHeaders(_, _))
            .WillByDefault(Return(lstRequiredHeaders));

    MockIMessage objMessageRequiresRpr;
    ON_CALL(objMessageRequiresRpr, GetMessage)
            .WillByDefault(Return(&objSipMessageRequiresRpr));

    EXPECT_TRUE(objExtensionSet.IsSupportRequiredExtensions(objMessageRequiresRpr));
}

TEST(MtcExtensionSetTest, IsSupportRequiredExtensionsReturnsFalseForNotAvailableExtension)
{
    MtcExtensionSet objExtensionSet = CreateExtensionSetSupportsRprOnly();

    ImsList<AString> lstRequiredHeaders;
    lstRequiredHeaders.Append(MtcExtensionSet::OPTION_TAG_TIMER);

    MockISipMessage objSipMessageRequiresTimer;
    ON_CALL(objSipMessageRequiresTimer, GetHeaders(_, _))
            .WillByDefault(Return(lstRequiredHeaders));

    MockIMessage objMessageRequiresTimer;
    ON_CALL(objMessageRequiresTimer, GetMessage)
            .WillByDefault(Return(&objSipMessageRequiresTimer));

    EXPECT_FALSE(objExtensionSet.IsSupportRequiredExtensions(objMessageRequiresTimer));
}

MockIMtcExtension* CreateMockIMtcExtension(IN const IMS_CHAR* pszOptionTag)
{
    MockIMtcExtension* pExtension = new MockIMtcExtension();

    AString strOptionTag(pszOptionTag);
    ON_CALL(*pExtension, GetOptionTag)
            .WillByDefault(ReturnRef(strOptionTag));

    return pExtension;
}

TEST(MtcExtensionSetTest, FormatRequestCallsEachExtensions)
{
    IMS_UINT32 nMethod = IMessage::SESSION_START;
    MockIMessage objMessage;

    MockIMtcExtension* pExtension = CreateMockIMtcExtension("any");
    EXPECT_CALL(*pExtension, FormatRequest(nMethod, Ref(objMessage)))
            .Times(1);

    ImsList<IMtcExtension*> lstExtensions;
    lstExtensions.Append(pExtension);

    MtcExtensionSet objExtensionSet(lstExtensions);
    objExtensionSet.FormatRequest(nMethod, objMessage);
}

TEST(MtcExtensionSetTest, FormatResponseCallsEachExtensions)
{
    IMS_UINT32 nMethod = IMessage::SESSION_START;
    MockIMessage objMessage;

    MockIMtcExtension* pExtension = CreateMockIMtcExtension("any");
    EXPECT_CALL(*pExtension, FormatResponse(nMethod, Ref(objMessage)))
            .Times(1);

    ImsList<IMtcExtension*> lstExtensions;
    lstExtensions.Append(pExtension);

    MtcExtensionSet objExtensionSet(lstExtensions);
    objExtensionSet.FormatResponse(nMethod, objMessage);
}

TEST(MtcExtensionSetTest, HandleRequestCallsEachExtensions)
{
    IMS_UINT32 nMethod = IMessage::SESSION_START;
    MockIMessage objMessage;

    MockIMtcExtension* pExtension = CreateMockIMtcExtension("any");
    EXPECT_CALL(*pExtension, HandleRequest(nMethod, Ref(objMessage)))
            .Times(1);

    ImsList<IMtcExtension*> lstExtensions;
    lstExtensions.Append(pExtension);

    MtcExtensionSet objExtensionSet(lstExtensions);
    objExtensionSet.HandleRequest(nMethod, objMessage);
}

TEST(MtcExtensionSetTest, HandleResponseCallsEachExtensions)
{
    IMS_UINT32 nMethod = IMessage::SESSION_START;
    MockIMessage objMessage;

    MockIMtcExtension* pExtension = CreateMockIMtcExtension("any");
    EXPECT_CALL(*pExtension, HandleResponse(nMethod, Ref(objMessage)))
            .Times(1);

    ImsList<IMtcExtension*> lstExtensions;
    lstExtensions.Append(pExtension);

    MtcExtensionSet objExtensionSet(lstExtensions);
    objExtensionSet.HandleResponse(nMethod, objMessage);
}
