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

#include "AString.h"
#include "AStringBuffer.h"
#include "ussi/UssiConstants.h"
#include "ussi/UssiDataCreator.h"
#include <gtest/gtest.h>

namespace android
{

LOCAL const AString USSD_STRING("any ussd string");
LOCAL const AString USSD_ELEMENT("<ussd-string>any ussd string</ussd-string>");
LOCAL const AString ERROR_ELEMENT_1("<error-code>1</error-code>");
LOCAL const AString ERROR_ELEMENT_2("<error-code>2</error-code>");
LOCAL const AString ERROR_ELEMENT_3("<error-code>3</error-code>");
LOCAL const AString ERROR_ELEMENT_4("<error-code>4</error-code>");

TEST(UssiDataCreatorTest, GetXmlBodyWithEmptyUssdStringDoesNothing)
{
    AStringBuffer objXml(UssiConstants::XML_BUFFER_SIZE);
    UssiDataCreator::GetXmlBody(
            AString::ConstEmpty(), objXml, UssiModeType::NONE, UssiError::CODE_NONE);

    EXPECT_EQ(objXml.GetLength(), 0);
}

TEST(UssiDataCreatorTest, GetXmlBodyWithModeTypeNone)
{
    AStringBuffer objXml(UssiConstants::XML_BUFFER_SIZE);
    UssiDataCreator::GetXmlBody(USSD_STRING, objXml, UssiModeType::NONE, UssiError::CODE_NONE);

    EXPECT_TRUE(objXml.GetString().Contains(USSD_ELEMENT));
    EXPECT_FALSE(objXml.GetString().Contains(UssiConstants::ELEMENT_ERROR_CODE));
    EXPECT_FALSE(objXml.GetString().Contains(UssiConstants::ELEMENT_USS_REQUEST));
    EXPECT_FALSE(objXml.GetString().Contains(UssiConstants::ELEMENT_USS_NOTIFY));
}

TEST(UssiDataCreatorTest, GetXmlBodyWithErrorCode)
{
    AStringBuffer objXml(UssiConstants::XML_BUFFER_SIZE);

    UssiDataCreator::GetXmlBody(USSD_STRING, objXml, UssiModeType::NONE, UssiError::CODE_1);
    EXPECT_TRUE(objXml.GetString().Contains(USSD_ELEMENT));
    EXPECT_TRUE(objXml.GetString().Contains(ERROR_ELEMENT_1));

    UssiDataCreator::GetXmlBody(USSD_STRING, objXml, UssiModeType::NONE, UssiError::CODE_2);
    EXPECT_TRUE(objXml.GetString().Contains(ERROR_ELEMENT_2));

    UssiDataCreator::GetXmlBody(USSD_STRING, objXml, UssiModeType::NONE, UssiError::CODE_3);
    EXPECT_TRUE(objXml.GetString().Contains(ERROR_ELEMENT_3));

    UssiDataCreator::GetXmlBody(USSD_STRING, objXml, UssiModeType::NONE, UssiError::CODE_4);
    EXPECT_TRUE(objXml.GetString().Contains(ERROR_ELEMENT_4));
}

TEST(UssiDataCreatorTest, GetXmlBodyWithModeTypeRequest)
{
    AStringBuffer objXml(UssiConstants::XML_BUFFER_SIZE);
    UssiDataCreator::GetXmlBody(USSD_STRING, objXml, UssiModeType::REQUEST, UssiError::CODE_NONE);

    EXPECT_TRUE(objXml.GetString().Contains(USSD_ELEMENT));
    EXPECT_TRUE(objXml.GetString().Contains(UssiConstants::ELEMENT_USS_REQUEST));
    EXPECT_FALSE(objXml.GetString().Contains(UssiConstants::ELEMENT_USS_NOTIFY));
}

TEST(UssiDataCreatorTest, GetXmlBodyWithModeTypeNotify)
{
    AStringBuffer objXml(UssiConstants::XML_BUFFER_SIZE);
    UssiDataCreator::GetXmlBody(USSD_STRING, objXml, UssiModeType::NOTIFY, UssiError::CODE_NONE);

    EXPECT_TRUE(objXml.GetString().Contains(USSD_ELEMENT));
    EXPECT_TRUE(objXml.GetString().Contains(UssiConstants::ELEMENT_USS_NOTIFY));
    EXPECT_FALSE(objXml.GetString().Contains(UssiConstants::ELEMENT_USS_REQUEST));
}

}  // namespace android
