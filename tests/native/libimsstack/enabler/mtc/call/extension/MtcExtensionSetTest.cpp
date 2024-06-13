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

#include "IMessage.h"
#include "ISipMessage.h"
#include "ImsList.h"
#include "call/MockIMtcCallContext.h"
#include "call/extension/IMtcExtension.h"
#include "call/extension/MockIMtcExtension.h"
#include "call/extension/MtcExtension.h"
#include "call/extension/MtcExtensionSet.h"
#include "core/MockIMessage.h"
#include "sipcore/MockISipMessage.h"
#include "utility/MessageUtils.h"
#include "utility/MockIMessageUtils.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::Ref;
using ::testing::Return;
using ::testing::ReturnRef;

const LOCAL AString OPTION_TAG("some_tag");

MtcExtensionSet CreateExtensionSetSupportsRprOnly(IN IMtcCallContext& objContext)
{
    ImsList<IMtcExtension*> lstExtensions;
    lstExtensions.Append(new MtcExtension(objContext, MtcExtensionSet::OPTION_TAG_RPR, {}, {}));

    return MtcExtensionSet(objContext, lstExtensions);
}

MtcExtensionSet CreateExtensionSetSupportsTdialogOnly(IN IMtcCallContext& objContext)
{
    ImsList<IMtcExtension*> lstExtensions;
    lstExtensions.Append(
            new MtcExtension(objContext, MtcExtensionSet::OPTION_TAG_TARGET_DIALOG, {}, {}));

    return MtcExtensionSet(objContext, lstExtensions);
}

MockIMtcExtension* CreateMockIMtcExtension(IN const AString& strOptionTag)
{
    MockIMtcExtension* pExtension = new MockIMtcExtension();

    ON_CALL(*pExtension, GetOptionTag).WillByDefault(ReturnRef(strOptionTag));

    return pExtension;
}

TEST(MtcExtensionSetTest, ConstructorWithEmptyOptionTags)
{
    MockIMtcCallContext objContext;
    ImsList<IMtcExtension*> lstEmptyExtensions;
    MtcExtensionSet objExtensionSet(objContext, lstEmptyExtensions);

    EXPECT_FALSE(objExtensionSet.IsAvailableOnLocal(
            MtcExtensionSet::OPTION_TAG_EARLY_DIALOG_TERMINATED));
    EXPECT_FALSE(objExtensionSet.IsAvailableOnLocal(MtcExtensionSet::OPTION_TAG_FROM_CHANGE));
    EXPECT_FALSE(objExtensionSet.IsAvailableOnLocal(MtcExtensionSet::OPTION_TAG_HISTORY_INFO));
    EXPECT_FALSE(objExtensionSet.IsAvailableOnLocal(MtcExtensionSet::OPTION_TAG_PRECONDITION));
    EXPECT_FALSE(objExtensionSet.IsAvailableOnLocal(MtcExtensionSet::OPTION_TAG_REPLACES));
    EXPECT_FALSE(objExtensionSet.IsAvailableOnLocal(MtcExtensionSet::OPTION_TAG_RPR));
    EXPECT_FALSE(objExtensionSet.IsAvailableOnLocal(MtcExtensionSet::OPTION_TAG_TARGET_DIALOG));
}

TEST(MtcExtensionSetTest, CopyConstructor)
{
    MockIMtcCallContext objContext;
    MtcExtensionSet objExtensionSet = CreateExtensionSetSupportsRprOnly(objContext);
    // This is a test to verify the copy constructor.
    // NOLINTNEXTLINE(performance-unnecessary-copy-initialization)
    MtcExtensionSet objCopiedExtensionSet(objExtensionSet);

    EXPECT_EQ(objExtensionSet.IsAvailableOnLocal(MtcExtensionSet::OPTION_TAG_RPR),
            objCopiedExtensionSet.IsAvailableOnLocal(MtcExtensionSet::OPTION_TAG_RPR));
    EXPECT_EQ(objExtensionSet.IsAvailableOnLocal(MtcExtensionSet::OPTION_TAG_TARGET_DIALOG),
            objCopiedExtensionSet.IsAvailableOnLocal(MtcExtensionSet::OPTION_TAG_TARGET_DIALOG));
}

TEST(MtcExtensionSetTest, AssignmentOperator)
{
    MockIMtcCallContext objContext;
    MtcExtensionSet objAssignedExtensionSet = CreateExtensionSetSupportsTdialogOnly(objContext);
    MtcExtensionSet objExtensionSet = CreateExtensionSetSupportsRprOnly(objContext);

    objAssignedExtensionSet = objExtensionSet;

    EXPECT_TRUE(objAssignedExtensionSet.IsAvailableOnLocal(MtcExtensionSet::OPTION_TAG_RPR));
    EXPECT_FALSE(
            objAssignedExtensionSet.IsAvailableOnLocal(MtcExtensionSet::OPTION_TAG_TARGET_DIALOG));
}

TEST(MtcExtensionSetTest, IsAvailableOnBothInitiallyReturnsFalse)
{
    MockIMtcCallContext objContext;
    MtcExtensionSet objExtensionSet = CreateExtensionSetSupportsRprOnly(objContext);

    EXPECT_FALSE(objExtensionSet.IsAvailableOnBoth(MtcExtensionSet::OPTION_TAG_RPR));
    EXPECT_FALSE(objExtensionSet.IsAvailableOnBoth(MtcExtensionSet::OPTION_TAG_TARGET_DIALOG));
}

TEST(MtcExtensionSetTest, IsAvailableOnLocalInitiallyReturnsInitialValue)
{
    MockIMtcCallContext objContext;
    MtcExtensionSet objExtensionSet = CreateExtensionSetSupportsRprOnly(objContext);

    EXPECT_TRUE(objExtensionSet.IsAvailableOnLocal(MtcExtensionSet::OPTION_TAG_RPR));
    EXPECT_FALSE(objExtensionSet.IsAvailableOnLocal(MtcExtensionSet::OPTION_TAG_TARGET_DIALOG));
}

TEST(MtcExtensionSetTest, IsRequiredOnRemoteReturnsFalseIfLocalNotSupported)
{
    MockIMtcCallContext objContext;
    MtcExtensionSet objExtensionSet = CreateExtensionSetSupportsRprOnly(objContext);

    EXPECT_FALSE(objExtensionSet.IsRequiredOnRemote(OPTION_TAG));
}

TEST(MtcExtensionSetTest, IsRequiredOnRemoteReturnsFalseIfRemoteNotRequires)
{
    MockIMtcExtension* pExtension = CreateMockIMtcExtension(OPTION_TAG);
    ON_CALL(*pExtension, IsRequiredOnRemote).WillByDefault(Return(IMS_FALSE));

    ImsList<IMtcExtension*> lstExtensions;
    lstExtensions.Append(pExtension);

    MockIMtcCallContext objContext;
    MtcExtensionSet objExtensionSet(objContext, lstExtensions);

    EXPECT_FALSE(objExtensionSet.IsRequiredOnRemote(OPTION_TAG));
}

TEST(MtcExtensionSetTest, IsRequiredOnRemoteReturnsTrueIfRemoteRequires)
{
    MockIMtcExtension* pExtension = CreateMockIMtcExtension(OPTION_TAG);
    ON_CALL(*pExtension, IsRequiredOnRemote).WillByDefault(Return(IMS_TRUE));

    ImsList<IMtcExtension*> lstExtensions;
    lstExtensions.Append(pExtension);

    MockIMtcCallContext objContext;
    MtcExtensionSet objExtensionSet(objContext, lstExtensions);

    EXPECT_TRUE(objExtensionSet.IsRequiredOnRemote(OPTION_TAG));
}

TEST(MtcExtensionSetTest, IsSupportRequiredExtensionsReturnsTrueForNotAvailableExtension)
{
    MockIMtcCallContext objContext;
    MockIMessageUtils objMessageUtils;
    ON_CALL(objContext, GetMessageUtils).WillByDefault(ReturnRef(objMessageUtils));

    MtcExtensionSet objExtensionSet = CreateExtensionSetSupportsRprOnly(objContext);

    ImsList<AString> lstRequiredHeaders;
    lstRequiredHeaders.Append(MtcExtensionSet::OPTION_TAG_RPR);

    MockISipMessage objSipMessageRequiresRpr;
    ON_CALL(objSipMessageRequiresRpr, GetHeaders(_, _)).WillByDefault(Return(lstRequiredHeaders));

    MockIMessage objMessageRequiresRpr;
    ON_CALL(objMessageRequiresRpr, GetMessage).WillByDefault(Return(&objSipMessageRequiresRpr));

    AString strNotSupportedExtension;
    EXPECT_TRUE(objExtensionSet.IsSupportRequiredExtensions(
            objMessageRequiresRpr, strNotSupportedExtension));
    EXPECT_TRUE(strNotSupportedExtension.GetLength() == 0);
}

TEST(MtcExtensionSetTest, IsSupportRequiredExtensionsReturnsFalseForNotAvailableExtension)
{
    MockIMtcCallContext objContext;
    MessageUtils objMessageUtils;
    ON_CALL(objContext, GetMessageUtils).WillByDefault(ReturnRef(objMessageUtils));

    MtcExtensionSet objExtensionSet = CreateExtensionSetSupportsRprOnly(objContext);

    ImsList<AString> lstRequiredHeaders;
    lstRequiredHeaders.Append(MtcExtensionSet::OPTION_TAG_TARGET_DIALOG);

    MockISipMessage objSipMessageRequiresTdialog;
    ON_CALL(objSipMessageRequiresTdialog, GetHeaders(_, _))
            .WillByDefault(Return(lstRequiredHeaders));

    MockIMessage objMessageRequiresTdialog;
    ON_CALL(objMessageRequiresTdialog, GetMessage)
            .WillByDefault(Return(&objSipMessageRequiresTdialog));

    AString strNotSupportedExtension;
    EXPECT_FALSE(objExtensionSet.IsSupportRequiredExtensions(
            objMessageRequiresTdialog, strNotSupportedExtension));
    EXPECT_EQ(strNotSupportedExtension, MtcExtensionSet::OPTION_TAG_TARGET_DIALOG);
}

TEST(MtcExtensionSetTest, FormatRequestCallsEachExtensions)
{
    RequestType eType = RequestType::START;
    MockIMessage objMessage;

    MockIMtcExtension* pExtension = CreateMockIMtcExtension(OPTION_TAG);
    EXPECT_CALL(*pExtension, FormatRequest(eType, Ref(objMessage))).Times(1);

    ImsList<IMtcExtension*> lstExtensions;
    lstExtensions.Append(pExtension);

    MockIMtcCallContext objContext;
    MtcExtensionSet objExtensionSet(objContext, lstExtensions);
    objExtensionSet.FormatRequest(eType, objMessage);
}

TEST(MtcExtensionSetTest, FormatResponseCallsEachExtensions)
{
    ResponseType eType = ResponseType::PROVISIONAL_RESPONSE;
    MockIMessage objMessage;

    MockIMtcExtension* pExtension = CreateMockIMtcExtension(OPTION_TAG);
    EXPECT_CALL(*pExtension, FormatResponse(eType, Ref(objMessage))).Times(1);

    ImsList<IMtcExtension*> lstExtensions;
    lstExtensions.Append(pExtension);

    MockIMtcCallContext objContext;
    MtcExtensionSet objExtensionSet(objContext, lstExtensions);
    objExtensionSet.FormatResponse(eType, objMessage);
}

TEST(MtcExtensionSetTest, HandleRequestCallsEachExtensions)
{
    RequestType eType = RequestType::START;
    MockIMessage objMessage;

    MockIMtcExtension* pExtension = CreateMockIMtcExtension(OPTION_TAG);
    EXPECT_CALL(*pExtension, HandleRequest(eType, Ref(objMessage))).Times(1);

    ImsList<IMtcExtension*> lstExtensions;
    lstExtensions.Append(pExtension);

    MockIMtcCallContext objContext;
    MtcExtensionSet objExtensionSet(objContext, lstExtensions);
    objExtensionSet.HandleRequest(eType, objMessage);
}

TEST(MtcExtensionSetTest, HandleResponseCallsEachExtensions)
{
    ResponseType eType = ResponseType::PROVISIONAL_RESPONSE;
    MockIMessage objMessage;

    MockIMtcExtension* pExtension = CreateMockIMtcExtension(OPTION_TAG);
    EXPECT_CALL(*pExtension, HandleResponse(eType, Ref(objMessage))).Times(1);

    ImsList<IMtcExtension*> lstExtensions;
    lstExtensions.Append(pExtension);

    MockIMtcCallContext objContext;
    MtcExtensionSet objExtensionSet(objContext, lstExtensions);
    objExtensionSet.HandleResponse(eType, objMessage);
}
