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
#include <gtest/gtest.h>

#include "ImsStrLib.h"
#include "TextParser.h"

namespace android
{

class TextParserTest : public ::testing::Test
{
protected:
    virtual void SetUp() override {}

    virtual void TearDown() override {}
};

TEST_F(TextParserTest, BooleanToString)
{
    EXPECT_STREQ("true", TextParser::BooleanToString(IMS_TRUE));
    EXPECT_STREQ("TRUE", TextParser::BooleanToString(IMS_TRUE, IMS_FALSE));
    EXPECT_STREQ("false", TextParser::BooleanToString(IMS_FALSE));
    EXPECT_STREQ("FALSE", TextParser::BooleanToString(IMS_FALSE, IMS_FALSE));
}

TEST_F(TextParserTest, CharToHexString)
{
    EXPECT_STREQ("61", TextParser::CharToHexString('a').GetStr());
    EXPECT_STREQ("7A", TextParser::CharToHexString('z').GetStr());
    EXPECT_STREQ("41", TextParser::CharToHexString('A').GetStr());
    EXPECT_STREQ("5A", TextParser::CharToHexString('Z').GetStr());
    EXPECT_STREQ("20", TextParser::CharToHexString(' ').GetStr());
    EXPECT_STREQ("0A", TextParser::CharToHexString('\n').GetStr());
    EXPECT_STREQ("0D", TextParser::CharToHexString('\r').GetStr());
}

TEST_F(TextParserTest, HexStringToChar)
{
    EXPECT_EQ('a', TextParser::HexStringToChar("0x61"));
    EXPECT_EQ('a', TextParser::HexStringToChar("61"));
    EXPECT_EQ(-1, TextParser::HexStringToChar("0x6"));
    EXPECT_EQ(-1, TextParser::HexStringToChar("6"));

    EXPECT_EQ('z', TextParser::HexStringToChar("0x7A"));
    EXPECT_EQ('z', TextParser::HexStringToChar("0x7a"));
    EXPECT_EQ('z', TextParser::HexStringToChar("7A"));
    EXPECT_EQ(-1, TextParser::HexStringToChar("0xA"));
    EXPECT_EQ(-1, TextParser::HexStringToChar("A"));
}

TEST_F(TextParserTest, IsTokenCharacter)
{
    // 0x00 ~ 0x20, 0x7F
    // clang-format off
    const IMS_CHAR acNonTokenChars[] = {
            0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,
            0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13,
            0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D,
            0x1E, 0x1F, 0x20, 0x7F,
            '(', ')', '<', '>', '@', ',', ';', ':','\\', '/', '"', '[', ']', '?', '='
    };
    // clang-format on
    const IMS_CHAR* pszTokenChars =
            "!#$%&'*+-.0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ^_`abcdefghijklmnopqrstuvwxyz{|}~";

    // Token characters
    IMS_SINT32 nTokenCharsLen = IMS_StrLen(pszTokenChars);

    for (IMS_SINT32 i = 0; i < nTokenCharsLen; ++i)
    {
        ASSERT_TRUE(TextParser::IsTokenCharacter(pszTokenChars[i]));
    }

    // Non-token characters
    IMS_SINT32 nNonTokenCharsLen = sizeof(acNonTokenChars) / sizeof(IMS_CHAR);

    for (IMS_SINT32 i = 0; i < nNonTokenCharsLen; ++i)
    {
        ASSERT_FALSE(TextParser::IsTokenCharacter(acNonTokenChars[i]));
    }
}

TEST_F(TextParserTest, IsValidMediaType)
{
    ASSERT_FALSE(TextParser::IsValidMediaType(""));
    ASSERT_FALSE(TextParser::IsValidMediaType("application"));
    ASSERT_FALSE(TextParser::IsValidMediaType("application/"));
    ASSERT_FALSE(TextParser::IsValidMediaType("applic:ation/xml"));
    ASSERT_FALSE(TextParser::IsValidMediaType("application/xm?l"));
    ASSERT_FALSE(TextParser::IsValidMediaType("application/<xml>"));
    ASSERT_FALSE(TextParser::IsValidMediaType("[application]/xml"));
    ASSERT_FALSE(TextParser::IsValidMediaType("application/ xml"));
    ASSERT_FALSE(TextParser::IsValidMediaType("application /xml"));
    ASSERT_FALSE(TextParser::IsValidMediaType("application/xml;param=p:value"));

    ASSERT_TRUE(TextParser::IsValidMediaType("application/xml"));
    ASSERT_TRUE(TextParser::IsValidMediaType("application/xml;"));
    ASSERT_TRUE(TextParser::IsValidMediaType("application/xml; param=pvalue"));
    ASSERT_TRUE(TextParser::IsValidMediaType("application/xml-doc"));
    ASSERT_TRUE(TextParser::IsValidMediaType("application/xml-doc;param=pvalue"));
    ASSERT_TRUE(TextParser::IsValidMediaType("application/xml-doc;param=pvalue;param2=pvalue2"));
}

TEST_F(TextParserTest, DoPercentDecoding)
{
    AString strData("Hello, Mr. IMS~!!");
    AString strPeData("Hello%2C%20Mr%2E IMS~%21!");

    ASSERT_TRUE(strData.Equals(TextParser::DoPercentDecoding(strData)));
    ASSERT_TRUE(strData.Equals(TextParser::DoPercentDecoding(strPeData)));
}

TEST_F(TextParserTest, DoPercentEncoding)
{
    AString strData("Hello, @ Mr. IMS~!!");
    AString strPeData("Hello%2C %40 Mr. IMS~%21%21");

    ASSERT_TRUE(strPeData.Equals(TextParser::DoPercentEncoding(strData)));

    AString strPeExtraChar(".~");
    AString strPeExcludingChar(",@");
    strPeData = "Hello, @ Mr%2E IMS%7E%21%21";

    ASSERT_TRUE(strPeData.Equals(
            TextParser::DoPercentEncoding(strData, strPeExtraChar, strPeExcludingChar)));

    strData = "Hello~ Mr. IMS.";
    ASSERT_TRUE(strData.Equals(TextParser::DoPercentEncoding(strData)));
}

TEST_F(TextParserTest, DoPercentEncodingEx)
{
    AString strData("Hello, @ Mr. IMS~!!");
    AString strPeData("Hello%2C%20%40%20Mr%2E%20IMS%7E%21%21");

    ASSERT_TRUE(strPeData.Equals(TextParser::DoPercentEncodingEx(strData, AString::ConstNull())));

    AString strPeExcludingChar(",@!");
    strPeData = "Hello,%20@%20Mr%2E%20IMS%7E!!";

    ASSERT_TRUE(strPeData.Equals(TextParser::DoPercentEncodingEx(strData, strPeExcludingChar)));

    strData = "Hello1234MrIMS";
    ASSERT_TRUE(strData.Equals(TextParser::DoPercentEncodingEx(strData, AString::ConstNull())));
}

TEST_F(TextParserTest, GetIndexOfDelimiter)
{
    AString strValue("my-name <ims-ims>");

    EXPECT_EQ(2, TextParser::GetIndexOfDelimiter(strValue, '-'));
    EXPECT_EQ(2, TextParser::GetIndexOfDelimiter(strValue, '-', IMS_FALSE));
    EXPECT_EQ(AString::NPOS, TextParser::GetIndexOfDelimiter(strValue, '?'));
    EXPECT_EQ(AString::NPOS, TextParser::GetIndexOfDelimiter(strValue, '?', IMS_FALSE));

    strValue = "\"my-name\" <ims-ims>";

    EXPECT_EQ(14, TextParser::GetIndexOfDelimiter(strValue, '-'));
    EXPECT_EQ(3, TextParser::GetIndexOfDelimiter(strValue, '-', IMS_FALSE));
    EXPECT_EQ(AString::NPOS, TextParser::GetIndexOfDelimiter(strValue, 'n'));
    EXPECT_EQ(4, TextParser::GetIndexOfDelimiter(strValue, 'n', IMS_FALSE));
}

TEST_F(TextParserTest, ParseMediaType)
{
    AString strType;
    AString strSubType;

    ASSERT_FALSE(TextParser::ParseMediaType("application", strType, strSubType));
    ASSERT_TRUE(strType.IsNull());
    ASSERT_TRUE(strSubType.IsNull());

    ASSERT_FALSE(TextParser::ParseMediaType("application/", strType, strSubType));
    ASSERT_TRUE(strType.IsNull());
    ASSERT_TRUE(strSubType.IsNull());

    ASSERT_FALSE(TextParser::ParseMediaType("/xml", strType, strSubType));
    ASSERT_TRUE(strType.IsNull());
    ASSERT_TRUE(strSubType.IsNull());

    ASSERT_TRUE(TextParser::ParseMediaType("application/xml", strType, strSubType));
    ASSERT_TRUE(strType.Equals("application"));
    ASSERT_TRUE(strSubType.Equals("xml"));

    strType = AString::ConstNull();
    strSubType = AString::ConstNull();

    ASSERT_TRUE(TextParser::ParseMediaType("application/ xml", strType, strSubType));
    ASSERT_TRUE(strType.Equals("application"));
    ASSERT_TRUE(strSubType.Equals(" xml"));

    strType = AString::ConstNull();
    strSubType = AString::ConstNull();

    ASSERT_TRUE(TextParser::ParseMediaType("application/xml;param=value", strType, strSubType));
    ASSERT_TRUE(strType.Equals("application"));
    ASSERT_TRUE(strSubType.Equals("xml"));

    strType = AString::ConstNull();
    strSubType = AString::ConstNull();

    ASSERT_TRUE(TextParser::ParseMediaType("application/xml ;param=value", strType, strSubType));
    ASSERT_TRUE(strType.Equals("application"));
    ASSERT_TRUE(strSubType.Equals("xml "));
}

TEST_F(TextParserTest, TrimDquot)
{
    AString strNonDquoted = "myname";
    AString strDquoted = "\"myname\"";

    ASSERT_TRUE(strNonDquoted.Equals(TextParser::TrimDquot(strNonDquoted)));
    ASSERT_TRUE(strNonDquoted.Equals(TextParser::TrimDquot(strDquoted)));

    strNonDquoted = "\"myname";
    ASSERT_TRUE(strNonDquoted.Equals(TextParser::TrimDquot(strNonDquoted)));
}

}  // namespace android
