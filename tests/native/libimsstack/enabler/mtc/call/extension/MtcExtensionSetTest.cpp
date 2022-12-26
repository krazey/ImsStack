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
#include "MockIMtcContext.h"
#include "MtcContextRepository.h"
#include "call/extension/IMtcExtension.h"
#include "call/extension/MockIMtcExtension.h"
#include "call/extension/MtcExtension.h"
#include "call/extension/MtcExtensionSet.h"
#include "call/extension/RprExtension.h"
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

const AString strSomeOptionTag("some_tag");

MtcExtensionSet CreateExtensionSetSupportsRprOnly()
{
    ImsList<IMtcExtension*> lstExtensions;
    lstExtensions.Append(new RprExtension());

    return MtcExtensionSet(lstExtensions);
}

MtcExtensionSet CreateExtensionSetSupportsTdialogOnly()
{
    ImsList<IMtcExtension*> lstExtensions;
    lstExtensions.Append(new MtcExtension(MtcExtensionSet::OPTION_TAG_TARGET_DIALOG));

    return MtcExtensionSet(lstExtensions);
}

MockIMtcExtension* CreateMockIMtcExtension(IN const AString& strOptionTag)
{
    MockIMtcExtension* pExtension = new MockIMtcExtension();

    ON_CALL(*pExtension, GetOptionTag).WillByDefault(ReturnRef(strOptionTag));

    return pExtension;
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
}

TEST(MtcExtensionSetTest, CopyConstructor)
{
    MtcExtensionSet objExtensionSet = CreateExtensionSetSupportsRprOnly();
    MtcExtensionSet objCopiedExtensionSet(objExtensionSet);

    EXPECT_EQ(objExtensionSet.IsAvailableOnLocal(MtcExtensionSet::OPTION_TAG_RPR),
            objCopiedExtensionSet.IsAvailableOnLocal(MtcExtensionSet::OPTION_TAG_RPR));
    EXPECT_EQ(objExtensionSet.IsAvailableOnLocal(MtcExtensionSet::OPTION_TAG_TARGET_DIALOG),
            objCopiedExtensionSet.IsAvailableOnLocal(MtcExtensionSet::OPTION_TAG_TARGET_DIALOG));
}

TEST(MtcExtensionSetTest, AssignOperator)
{
    MtcExtensionSet objAssignedExtensionSet = CreateExtensionSetSupportsTdialogOnly();
    MtcExtensionSet objExtensionSet = CreateExtensionSetSupportsRprOnly();

    objAssignedExtensionSet = objExtensionSet;

    EXPECT_TRUE(objAssignedExtensionSet.IsAvailableOnLocal(MtcExtensionSet::OPTION_TAG_RPR));
    EXPECT_FALSE(
            objAssignedExtensionSet.IsAvailableOnLocal(MtcExtensionSet::OPTION_TAG_TARGET_DIALOG));
}

TEST(MtcExtensionSetTest, IsAvailableOnBothInitiallyReturnsFalse)
{
    MtcExtensionSet objExtensionSet = CreateExtensionSetSupportsRprOnly();

    EXPECT_FALSE(objExtensionSet.IsAvailableOnBoth(MtcExtensionSet::OPTION_TAG_RPR));
    EXPECT_FALSE(objExtensionSet.IsAvailableOnBoth(MtcExtensionSet::OPTION_TAG_TARGET_DIALOG));
}

TEST(MtcExtensionSetTest, IsAvailableOnLocalInitiallyReturnsInitialValue)
{
    MtcExtensionSet objExtensionSet = CreateExtensionSetSupportsRprOnly();

    EXPECT_TRUE(objExtensionSet.IsAvailableOnLocal(MtcExtensionSet::OPTION_TAG_RPR));
    EXPECT_FALSE(objExtensionSet.IsAvailableOnLocal(MtcExtensionSet::OPTION_TAG_TARGET_DIALOG));
}

TEST(MtcExtensionSetTest, IsSupportRequiredExtensionsReturnsTrueForNotAvailableExtension)
{
    MockIMtcContext objContext;
    MockIMessageUtils objMessageUtils;
    MtcContextRepository::GetInstance()->AddContext(IMS_SLOT_0, &objContext);
    ON_CALL(objContext, GetMessageUtils).WillByDefault(ReturnRef(objMessageUtils));

    MtcExtensionSet objExtensionSet = CreateExtensionSetSupportsRprOnly();

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
    MockIMtcContext objContext;
    MessageUtils objMessageUtils;
    MtcContextRepository::GetInstance()->AddContext(IMS_SLOT_0, &objContext);
    ON_CALL(objContext, GetMessageUtils).WillByDefault(ReturnRef(objMessageUtils));

    MtcExtensionSet objExtensionSet = CreateExtensionSetSupportsRprOnly();

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

    MockIMtcExtension* pExtension = CreateMockIMtcExtension(strSomeOptionTag);
    EXPECT_CALL(*pExtension, FormatRequest(eType, Ref(objMessage))).Times(1);

    ImsList<IMtcExtension*> lstExtensions;
    lstExtensions.Append(pExtension);

    MtcExtensionSet objExtensionSet(lstExtensions);
    objExtensionSet.FormatRequest(eType, objMessage);
}

TEST(MtcExtensionSetTest, FormatResponseCallsEachExtensions)
{
    ResponseType eType = ResponseType::PROVISIONAL_RESPONSE;
    MockIMessage objMessage;

    MockIMtcExtension* pExtension = CreateMockIMtcExtension(strSomeOptionTag);
    EXPECT_CALL(*pExtension, FormatResponse(eType, Ref(objMessage))).Times(1);

    ImsList<IMtcExtension*> lstExtensions;
    lstExtensions.Append(pExtension);

    MtcExtensionSet objExtensionSet(lstExtensions);
    objExtensionSet.FormatResponse(eType, objMessage);
}

TEST(MtcExtensionSetTest, HandleRequestCallsEachExtensions)
{
    RequestType eType = RequestType::START;
    MockIMessage objMessage;

    MockIMtcExtension* pExtension = CreateMockIMtcExtension(strSomeOptionTag);
    EXPECT_CALL(*pExtension, HandleRequest(eType, Ref(objMessage))).Times(1);

    ImsList<IMtcExtension*> lstExtensions;
    lstExtensions.Append(pExtension);

    MtcExtensionSet objExtensionSet(lstExtensions);
    objExtensionSet.HandleRequest(eType, objMessage);
}

TEST(MtcExtensionSetTest, HandleResponseCallsEachExtensions)
{
    ResponseType eType = ResponseType::PROVISIONAL_RESPONSE;
    MockIMessage objMessage;

    MockIMtcExtension* pExtension = CreateMockIMtcExtension(strSomeOptionTag);
    EXPECT_CALL(*pExtension, HandleResponse(eType, Ref(objMessage))).Times(1);

    ImsList<IMtcExtension*> lstExtensions;
    lstExtensions.Append(pExtension);

    MtcExtensionSet objExtensionSet(lstExtensions);
    objExtensionSet.HandleResponse(eType, objMessage);
}
