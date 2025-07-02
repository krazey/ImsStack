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

#include "ussi/UssiConstants.h"
#include "ussi/UssiData.h"
#include <gtest/gtest.h>

namespace android
{
// clang-format off
static const IMS_CHAR USSI_DATA_TEMPLATE[] = {
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
"<ussd-data>\n"
    "<language>_LANGUAGE_</language>\n"
    "<ussd-string>_USSD_STRING_</ussd-string>\n"
    "<error-code>_ERROR_CODE_</error-code>\n"
    "<anyExt>\n"
        "<_USS_TYPE_/>\n"
        "<alertingPattern>_ALERTING_PATTERN_</alertingPattern>\n"
    "</anyExt>\n"
"</ussd-data>\n"
};

static const IMS_CHAR EMPTY_USSI_DATA[] = {
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
"<ussd-data/>\n"
};
// clang-format on

class UssiDataTest : public ::testing::Test
{
public:
    inline UssiDataTest() :
            objUssiData(),
            strUssdString("default string"),
            strLanguage("en"),
            strErrorCode("0"),
            strUssType(UssiConstants::ELEMENT_USS_REQUEST),
            strAlertingPattern("0")
    {
    }
    inline ~UssiDataTest() {}

protected:
    UssiData objUssiData;

    AString strUssdString;
    AString strLanguage;
    AString strErrorCode;
    AString strUssType;
    AString strAlertingPattern;

    const AString GetUssiData()
    {
        AString strUssiData(USSI_DATA_TEMPLATE);
        strUssiData = strUssiData.Replace("_USSD_STRING_", strUssdString);
        strUssiData = strUssiData.Replace("_LANGUAGE_", strLanguage);
        strUssiData = strUssiData.Replace("_ERROR_CODE_", strErrorCode);
        strUssiData = strUssiData.Replace("_USS_TYPE_", strUssType);
        strUssiData = strUssiData.Replace("_ALERTING_PATTERN_", strAlertingPattern);

        return strUssiData;
    }

    const AString GetUssiDataWithInvalidElement(IN const AString& strName)
    {
        AString strInvalidName = "invalid-" + strName;
        AString strUssiData(USSI_DATA_TEMPLATE);
        strUssiData = strUssiData.Replace(strName, strInvalidName);
        return strUssiData;
    }
};

TEST_F(UssiDataTest, ParseSucceedsIfValidPackage)
{
    EXPECT_TRUE(objUssiData.Parse(GetUssiData()));
}

TEST_F(UssiDataTest, ParseFailsIfInvalidPackage)
{
    // empty package
    AString strXml;
    EXPECT_FALSE(objUssiData.Parse(strXml));

    // no conference-info element
    strXml = GetUssiDataWithInvalidElement("ussd-data");
    EXPECT_FALSE(objUssiData.Parse(strXml));

    // no child list
    EXPECT_FALSE(objUssiData.Parse(EMPTY_USSI_DATA));
}

TEST_F(UssiDataTest, GetLanguageReturnsValueFromXml)
{
    strLanguage = "added language";
    objUssiData.Parse(GetUssiData());
    EXPECT_EQ(objUssiData.GetLanguage(), strLanguage);
}

TEST_F(UssiDataTest, GetUssdStringReturnsValueFromXml)
{
    strUssdString = "added ussd string";
    objUssiData.Parse(GetUssiData());
    EXPECT_EQ(objUssiData.GetUssdString(), strUssdString);
}

TEST_F(UssiDataTest, GetErrorCodeReturnsValueFromXml)
{
    objUssiData.Parse(GetUssiData());
    EXPECT_EQ(objUssiData.GetErrorCode(), UssiError::CODE_NONE);

    strErrorCode = "1";
    objUssiData.Parse(GetUssiData());
    EXPECT_EQ(objUssiData.GetErrorCode(), UssiError::CODE_1);
}

TEST_F(UssiDataTest, GetUssiModeTypeReturnsNoneIfNoTypeExists)
{
    strUssType = "invalid";
    objUssiData.Parse(GetUssiData());
    EXPECT_EQ(objUssiData.GetAnyExtension().GetUssiModeType(), UssiModeType::NONE);
}

TEST_F(UssiDataTest, GetUssiModeTypeReturnsRequestIfRequestType)
{
    strUssType = UssiConstants::ELEMENT_USS_REQUEST;
    objUssiData.Parse(GetUssiData());
    EXPECT_EQ(objUssiData.GetAnyExtension().GetUssiModeType(), UssiModeType::REQUEST);
}

TEST_F(UssiDataTest, GetUssiModeTypeReturnsNotifyIfNotifyType)
{
    strUssType = UssiConstants::ELEMENT_USS_NOTIFY;
    objUssiData.Parse(GetUssiData());
    EXPECT_EQ(objUssiData.GetAnyExtension().GetUssiModeType(), UssiModeType::NOTIFY);
}

TEST_F(UssiDataTest, GetAlertingPatternReturnsValueFromXml)
{
    objUssiData.Parse(GetUssiData());
    EXPECT_EQ(objUssiData.GetAnyExtension().GetAlertingPattern(), 0);

    strAlertingPattern = "1";
    objUssiData.Parse(GetUssiData());
    EXPECT_EQ(objUssiData.GetAnyExtension().GetAlertingPattern(), 1);
}

TEST_F(UssiDataTest, ParseByParserReturnsNullIfParseFails)
{
    UssiDataParser objParser;
    AString strEmptyXml;
    const UssiData* pData = objParser.Parse(strEmptyXml);
    EXPECT_EQ(pData, nullptr);
}

TEST_F(UssiDataTest, ParseByParserReturnsUssiDataIfParseSucceeds)
{
    UssiDataParser objParser;
    UssiData* pData = objParser.Parse(GetUssiData());
    EXPECT_NE(pData, nullptr);
    delete pData;
}

}  // namespace android
