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
#ifndef TEXT_PARSER_H_
#define TEXT_PARSER_H_

#include "AString.h"

/**
 * @brief This class is the class for a global enumerations, definitions and constant variables.
 */
class TextParser
{
public:
    static const IMS_CHAR* BooleanToString(IN IMS_BOOL bValue, IN IMS_BOOL bLowerCase = IMS_TRUE);
    static AString CharToHexString(IN const IMS_CHAR c);
    static IMS_SINT32 HexStringToChar(IN const AString& strHex);
    static IMS_BOOL IsTokenCharacter(IN const IMS_CHAR c);
    static IMS_BOOL IsValidMediaType(IN const AString& strMediaType);

    // For percent-encoded data octet
    static AString DoPercentDecoding(IN const AString& strPeData);
    static AString DoPercentEncoding(IN const AString& strData,
            IN const AString& strPeExtraChar = AString::ConstNull(),
            IN const AString& strPeExcludingChar = AString::ConstNull());
    static AString DoPercentEncodingEx(IN const AString& strData,
            IN const AString& strPeExcludingChar /*alpha/num is included as default*/);

    static IMS_SINT32 GetIndexOfDelimiter(IN const AString& strValue, IN const IMS_CHAR cDelimiter,
            IN IMS_BOOL bCheckDquot = IMS_TRUE);
    static IMS_BOOL ParseMediaType(
            IN const AString& strMediaType, OUT AString& strType, OUT AString& strSubType);
    static AString TrimDquot(IN const AString& strValue);

public:
    // Constant values - General tokens (character values) for parsing
    static const IMS_CHAR CHAR_AMPERSAND;
    static const IMS_CHAR CHAR_ASTERISK;
    static const IMS_CHAR CHAR_AT;
    static const IMS_CHAR CHAR_BACKSLASH;
    static const IMS_CHAR CHAR_CR;
    static const IMS_CHAR CHAR_COLON;
    static const IMS_CHAR CHAR_COMMA;
    static const IMS_CHAR CHAR_DOT;
    static const IMS_CHAR CHAR_DQUOT;
    static const IMS_CHAR CHAR_EQUAL;
    static const IMS_CHAR CHAR_HYPHEN;
    static const IMS_CHAR CHAR_HTAB;
    static const IMS_CHAR CHAR_LAQUOT;
    static const IMS_CHAR CHAR_LF;
    static const IMS_CHAR CHAR_LSBRACKET;
    static const IMS_CHAR CHAR_PERCENT;
    static const IMS_CHAR CHAR_PLUS;
    static const IMS_CHAR CHAR_RAQUOT;
    static const IMS_CHAR CHAR_RSBRACKET;
    static const IMS_CHAR CHAR_QUESTIONMARK;
    static const IMS_CHAR CHAR_SEMICOLON;
    static const IMS_CHAR CHAR_SHARP;
    static const IMS_CHAR CHAR_SLASH;
    static const IMS_CHAR CHAR_SP;
    static const IMS_CHAR CHAR_UNDERSCORE;

    // Constant values - General tokens (string values) for parsing
    static const IMS_CHAR STR_ASTERISK[];
    static const IMS_CHAR STR_AT[];
    static const IMS_CHAR STR_BACKSLASH[];
    static const IMS_CHAR STR_CR[];
    static const IMS_CHAR STR_COLON[];
    static const IMS_CHAR STR_COMMA[];
    static const IMS_CHAR STR_DOT[];
    static const IMS_CHAR STR_DQUOTE[];
    static const IMS_CHAR STR_EQUAL[];
    static const IMS_CHAR STR_HYPHEN[];
    static const IMS_CHAR STR_HTAB[];
    static const IMS_CHAR STR_LAQUOT[];
    static const IMS_CHAR STR_LF[];
    static const IMS_CHAR STR_LSBRACKET[];
    static const IMS_CHAR STR_PERCENT[];
    static const IMS_CHAR STR_RAQUOT[];
    static const IMS_CHAR STR_RSBRACKET[];
    static const IMS_CHAR STR_QUESTIONMARK[];
    static const IMS_CHAR STR_SEMICOLON[];
    static const IMS_CHAR STR_SHARP[];
    static const IMS_CHAR STR_SLASH[];
    static const IMS_CHAR STR_SP[];
    static const IMS_CHAR STR_UNDERSCORE[];

    static const IMS_CHAR STR_CRLF[];
    static const IMS_CHAR STR_CRLFCRLF[];

    static const IMS_CHAR STR_TRUE[];
    static const IMS_CHAR STR_FALSE[];
    static const IMS_CHAR STR_SMALL_TRUE[];
    static const IMS_CHAR STR_SMALL_FALSE[];

private:
    static IMS_SINT32 GetExpectedCountOfUtf8Cont(IN const IMS_CHAR c);
    static IMS_BOOL IsHexCharacter(IN const IMS_CHAR c);
    static IMS_BOOL IsParameterString(IN const AString& strParameter);
    static IMS_BOOL IsParameterValueString(IN const AString& strParameterValue);
    static IMS_BOOL IsQdText(IN const IMS_CHAR c);
    static IMS_BOOL IsQuotedString(IN const AString& strQuotedString);
    static IMS_BOOL IsReservedCharacter(IN const IMS_CHAR c);
    static IMS_BOOL IsTokenString(IN const AString& strToken);
    static IMS_BOOL IsUtf8Cont(IN const IMS_CHAR c);
};

#endif
