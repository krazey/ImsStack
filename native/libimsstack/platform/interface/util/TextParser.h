/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090904  toastops@                 Created
    </table>

    Description

*/

#ifndef _TEXT_PARSER_H_
#define _TEXT_PARSER_H_

#include "AString.h"

class TextParser
{
public:
    static const IMS_CHAR* BooleanToString(IN IMS_BOOL bValue, IN IMS_BOOL bLowerCase = IMS_TRUE);
    static AString CharToHexString(IN CONST IMS_CHAR cChar);
    static IMS_SINT32 HexStringToChar(IN CONST AString& strHex);
    static IMS_BOOL IsTokenCharacter(IN CONST IMS_CHAR ch);
    static IMS_BOOL IsValidMediaType(IN CONST AString& strMediaType);

    // For percent-encoded data octet
    static AString DoPercentDecoding(IN CONST AString& strPEData);
    static AString DoPercentEncoding(IN CONST AString& strData,
            IN CONST AString& strPEExtraChar = AString::ConstNull(),
            IN CONST AString& strPEExcludingChar = AString::ConstNull());
    static AString DoPercentEncodingEx(IN CONST AString& strData,
            IN CONST AString& strPEExcludingChar /* alpha/num is included as default */);

    static IMS_SINT32 GetIndexOfDelimiter(IN CONST AString& strValue, IN CONST IMS_CHAR cDelimiter,
            IN IMS_BOOL bCheckDQUOT = IMS_TRUE);
    static IMS_BOOL ParseMediaType(
            IN CONST AString& strMediaType, OUT AString& strType, OUT AString& strSubType);
    static AString TrimDQUOT(IN CONST AString& strValue);

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
    static IMS_SINT32 GetExpectedCountOfUTF8Cont(IN CONST IMS_CHAR ch);
    static IMS_BOOL IsHexCharacter(IN CONST IMS_CHAR ch);
    static IMS_BOOL IsParameterString(IN CONST AString& strParameter);
    static IMS_BOOL IsParameterValueString(IN CONST AString& strParameterValue);
    static IMS_BOOL IsQdText(IN CONST IMS_CHAR ch);
    static IMS_BOOL IsQuotedString(IN CONST AString& strQuotedString);
    static IMS_BOOL IsReservedCharacter(IN CONST IMS_CHAR ch);
    static IMS_BOOL IsTokenString(IN CONST AString& strToken);
    static IMS_BOOL IsUTF8Cont(IN CONST IMS_CHAR ch);
};

#endif  // _TEXT_PARSER_H_
