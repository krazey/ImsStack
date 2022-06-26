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
#include "AStringBuffer.h"
#include "ImsLib.h"
#include "ServiceMemory.h"
#include "TextParser.h"

PUBLIC GLOBAL const IMS_CHAR TextParser::CHAR_AMPERSAND = '&';
PUBLIC GLOBAL const IMS_CHAR TextParser::CHAR_ASTERISK = '*';
PUBLIC GLOBAL const IMS_CHAR TextParser::CHAR_AT = '@';
PUBLIC GLOBAL const IMS_CHAR TextParser::CHAR_BACKSLASH = '\\';
PUBLIC GLOBAL const IMS_CHAR TextParser::CHAR_CR = '\r';
PUBLIC GLOBAL const IMS_CHAR TextParser::CHAR_COLON = ':';
PUBLIC GLOBAL const IMS_CHAR TextParser::CHAR_COMMA = ',';
PUBLIC GLOBAL const IMS_CHAR TextParser::CHAR_DOT = '.';
PUBLIC GLOBAL const IMS_CHAR TextParser::CHAR_DQUOT = '\"';
PUBLIC GLOBAL const IMS_CHAR TextParser::CHAR_EQUAL = '=';
PUBLIC GLOBAL const IMS_CHAR TextParser::CHAR_HYPHEN = '-';
PUBLIC GLOBAL const IMS_CHAR TextParser::CHAR_HTAB = '\t';
PUBLIC GLOBAL const IMS_CHAR TextParser::CHAR_LAQUOT = '<';
PUBLIC GLOBAL const IMS_CHAR TextParser::CHAR_LF = '\n';
PUBLIC GLOBAL const IMS_CHAR TextParser::CHAR_LSBRACKET = '[';
PUBLIC GLOBAL const IMS_CHAR TextParser::CHAR_PERCENT = '%';
PUBLIC GLOBAL const IMS_CHAR TextParser::CHAR_PLUS = '+';
PUBLIC GLOBAL const IMS_CHAR TextParser::CHAR_RAQUOT = '>';
PUBLIC GLOBAL const IMS_CHAR TextParser::CHAR_RSBRACKET = ']';
PUBLIC GLOBAL const IMS_CHAR TextParser::CHAR_QUESTIONMARK = '?';
PUBLIC GLOBAL const IMS_CHAR TextParser::CHAR_SEMICOLON = ';';
PUBLIC GLOBAL const IMS_CHAR TextParser::CHAR_SHARP = '#';
PUBLIC GLOBAL const IMS_CHAR TextParser::CHAR_SLASH = '/';
PUBLIC GLOBAL const IMS_CHAR TextParser::CHAR_SP = ' ';
PUBLIC GLOBAL const IMS_CHAR TextParser::CHAR_UNDERSCORE = '_';

PUBLIC GLOBAL const IMS_CHAR TextParser::STR_ASTERISK[] = "*";
PUBLIC GLOBAL const IMS_CHAR TextParser::STR_AT[] = "@";
PUBLIC GLOBAL const IMS_CHAR TextParser::STR_BACKSLASH[] = "\\";
PUBLIC GLOBAL const IMS_CHAR TextParser::STR_CR[] = "\r";
PUBLIC GLOBAL const IMS_CHAR TextParser::STR_COLON[] = ":";
PUBLIC GLOBAL const IMS_CHAR TextParser::STR_COMMA[] = ",";
PUBLIC GLOBAL const IMS_CHAR TextParser::STR_DOT[] = ".";
PUBLIC GLOBAL const IMS_CHAR TextParser::STR_DQUOTE[] = "\"";
PUBLIC GLOBAL const IMS_CHAR TextParser::STR_EQUAL[] = "=";
PUBLIC GLOBAL const IMS_CHAR TextParser::STR_HYPHEN[] = "-";
PUBLIC GLOBAL const IMS_CHAR TextParser::STR_HTAB[] = "\t";
PUBLIC GLOBAL const IMS_CHAR TextParser::STR_LAQUOT[] = "<";
PUBLIC GLOBAL const IMS_CHAR TextParser::STR_LF[] = "\n";
PUBLIC GLOBAL const IMS_CHAR TextParser::STR_LSBRACKET[] = "[";
PUBLIC GLOBAL const IMS_CHAR TextParser::STR_PERCENT[] = "%";
PUBLIC GLOBAL const IMS_CHAR TextParser::STR_RAQUOT[] = ">";
PUBLIC GLOBAL const IMS_CHAR TextParser::STR_RSBRACKET[] = "]";
PUBLIC GLOBAL const IMS_CHAR TextParser::STR_QUESTIONMARK[] = "?";
PUBLIC GLOBAL const IMS_CHAR TextParser::STR_SEMICOLON[] = ";";
PUBLIC GLOBAL const IMS_CHAR TextParser::STR_SHARP[] = "#";
PUBLIC GLOBAL const IMS_CHAR TextParser::STR_SLASH[] = "/";
PUBLIC GLOBAL const IMS_CHAR TextParser::STR_SP[] = " ";
PUBLIC GLOBAL const IMS_CHAR TextParser::STR_UNDERSCORE[] = "_";
PUBLIC GLOBAL const IMS_CHAR TextParser::STR_CRLF[] = "\r\n";
PUBLIC GLOBAL const IMS_CHAR TextParser::STR_CRLFCRLF[] = "\r\n\r\n";

PUBLIC GLOBAL const IMS_CHAR TextParser::STR_TRUE[] = "TRUE";
PUBLIC GLOBAL const IMS_CHAR TextParser::STR_FALSE[] = "FALSE";
PUBLIC GLOBAL const IMS_CHAR TextParser::STR_SMALL_TRUE[] = "true";
PUBLIC GLOBAL const IMS_CHAR TextParser::STR_SMALL_FALSE[] = "false";

PUBLIC GLOBAL const IMS_CHAR* TextParser::BooleanToString(
        IN IMS_BOOL bValue, IN IMS_BOOL bLowerCase /*= IMS_TRUE*/)
{
    if (bLowerCase)
    {
        return (bValue == IMS_TRUE) ? STR_SMALL_TRUE : STR_SMALL_FALSE;
    }
    else
    {
        return (bValue == IMS_TRUE) ? STR_TRUE : STR_FALSE;
    }
}

PUBLIC GLOBAL AString TextParser::CharToHexString(IN const IMS_CHAR c)
{
    AString strHex;
    return strHex.Sprintf("%02X", c);
}

PUBLIC GLOBAL IMS_SINT32 TextParser::HexStringToChar(IN const AString& strHex)
{
    AString strTmpHex;

    // HEX DIGIT with 0x prefix
    if (strHex.GetLength() == 4)
    {
        if ((strHex[0] == '0') && ((strHex[1] == 'x') || (strHex[1] == 'X')))
        {
            // Valid hex digit value
            strTmpHex = strHex.GetSubStr(2);
        }
        else
        {
            return (-1);
        }
    }
    // 2 HEX DIGIT only
    else if (strHex.GetLength() == 2)
    {
        strTmpHex = strHex;
    }
    else
    {
        return (-1);
    }

    if (!IsHexCharacter(strTmpHex[0]) || !IsHexCharacter(strTmpHex[1]))
    {
        return (-1);
    }

    IMS_CHAR c;
    const IMS_CHAR chFirst = strTmpHex[0];
    const IMS_CHAR chSecond = strTmpHex[1];

    if ((chFirst >= 'A') && (chFirst <= 'F'))
    {
        c = chFirst - 'A' + 10;
    }
    else if ((chFirst >= 'a') && (chFirst <= 'f'))
    {
        c = chFirst - 'a' + 10;
    }
    else
    {
        c = chFirst - '0';
    }

    c *= 16;

    if ((chSecond >= 'A') && (chSecond <= 'F'))
    {
        c += chSecond - 'A' + 10;
    }
    else if ((chSecond >= 'a') && (chSecond <= 'f'))
    {
        c += chSecond - 'a' + 10;
    }
    else
    {
        c += chSecond - '0';
    }

    return c;
}

// Compliant to RFC 2045
PUBLIC GLOBAL IMS_BOOL TextParser::IsTokenCharacter(IN const IMS_CHAR c)
{
    if (c == 0x21)
    {
        return IMS_TRUE;
    }

    if ((c >= 0x23) && (c <= 0x27))
    {
        return IMS_TRUE;
    }

    if ((c >= 0x2A) && (c <= 0x2B))
    {
        return IMS_TRUE;
    }

    if ((c >= 0x2D) && (c <= 0x2E))
    {
        return IMS_TRUE;
    }

    if ((c >= 0x30) && (c <= 0x39))
    {
        return IMS_TRUE;
    }

    if ((c >= 0x41) && (c <= 0x5A))
    {
        return IMS_TRUE;
    }

    if ((c >= 0x5E) && (c <= 0x7E))
    {
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

/**
 * @brief This method validates the MIME media types (in general, Content-Type header)
 *        according to RFC 2045 "MIME Part One: Format of Internet Message Bodies".
 */
PUBLIC GLOBAL IMS_BOOL TextParser::IsValidMediaType(IN const AString& strMediaType)
{
    // content := "Content-Type" ":" type "/" subtype *(";" parameter)
    // parameter := attribute "=" value

    if (strMediaType.IsNULL() || strMediaType.IsEmpty())
    {
        return IMS_FALSE;
    }

    // Finds a slash('/') character from the input value
    IMS_SINT32 nSlashIndex = strMediaType.GetIndexOf(CHAR_SLASH);

    if (nSlashIndex == AString::NPOS)
    {
        // systax error
        return IMS_FALSE;
    }

    AString strType = strMediaType.GetSubStr(0, nSlashIndex);

    if (!IsTokenString(strType))
    {
        return IMS_FALSE;
    }

    IMS_SINT32 nSemicolonIndex = strMediaType.GetIndexOf(CHAR_SEMICOLON, nSlashIndex + 1);

    if (nSemicolonIndex == AString::NPOS)
    {
        // No parameters
        return IsTokenString(strMediaType.GetSubStr(nSlashIndex + 1));
    }

    // Check the subtype
    AString strSubType =
            strMediaType.GetSubStr(nSlashIndex + 1, nSemicolonIndex - (nSlashIndex + 1));

    if (!IsTokenString(strSubType))
    {
        return IMS_FALSE;
    }

    // Check the parameters
    ImsList<AString> objParameters =
            strMediaType.GetSubStr(nSemicolonIndex + 1).Split(CHAR_SEMICOLON);

    for (IMS_UINT32 i = 0; i < objParameters.GetSize(); ++i)
    {
        if (!IsParameterString(objParameters.GetAt(i)))
        {
            return IMS_FALSE;
        }
    }

    return IMS_TRUE;
}

/**
 * @brief Converts a percent-encoded URI string to an original URI string.
 */
PUBLIC GLOBAL AString TextParser::DoPercentDecoding(IN const AString& strPeData)
{
    if (!strPeData.Contains(CHAR_PERCENT))
    {
        return strPeData;
    }

    const IMS_SINT32 nLength = strPeData.GetLength();
    AStringBuffer objBuffer(nLength);

    for (IMS_SINT32 i = 0; i < nLength; ++i)
    {
        if (strPeData[i] != CHAR_PERCENT)
        {
            objBuffer.Append(strPeData[i]);
        }
        else
        {
            AString strHexDigs = strPeData.GetSubStr(i + 1, 2);
            IMS_SINT32 c = HexStringToChar(strHexDigs);

            if ((c < 0x00) || (c > 0xFF))
            {
                objBuffer.Append(CHAR_PERCENT);
                objBuffer.Append(strHexDigs);
            }
            else
            {
                objBuffer.Append(static_cast<const IMS_CHAR>(c));
            }

            i += 2;
        }
    }

    return static_cast<const AStringBuffer&>(objBuffer).GetString();
}

/**
 * @brief Converts an original URI string to a percent-encoded URI string.
 *
 * @param strPeExtraChar The percent-encoded string including characters
 *                       from the non-reserved characters
 * @param strPeExcludingChar The percent-encoded string excluding characters
 *                           from the reserved characters
 */
PUBLIC GLOBAL AString TextParser::DoPercentEncoding(IN const AString& strData,
        IN const AString& strPeExtraChar /*= AString::ConstNull()*/,
        IN const AString& strPeExcludingChar /*= AString::ConstNull()*/)
{
    const IMS_SINT32 nLength = strData.GetLength();
    AStringBuffer objPeData(nLength * 2);
    AStringBuffer objHex(2);

    for (IMS_SINT32 i = 0; i < nLength; ++i)
    {
        if (!IsReservedCharacter(strData[i]))
        {
            if (!strPeExtraChar.IsNULL() && strPeExtraChar.Contains(strData[i]))
            {
                objHex.Sprintf("%02X", strData[i]);

                objPeData.Append(CHAR_PERCENT);
                objPeData.Append(objHex[0]);
                objPeData.Append(objHex[1]);
            }
            else
            {
                objPeData.Append(strData[i]);
            }
        }
        else
        {
            if (!strPeExcludingChar.IsNULL() && strPeExcludingChar.Contains(strData[i]))
            {
                objPeData.Append(strData[i]);
            }
            else
            {
                objHex.Sprintf("%02X", strData[i]);

                objPeData.Append(CHAR_PERCENT);
                objPeData.Append(objHex[0]);
                objPeData.Append(objHex[1]);
            }
        }
    }

    return static_cast<const AStringBuffer&>(objPeData).GetString();
}

/**
 * @brief Converts an original URI string to a percent-encoded URI string.
 *
 * @param strPeExcludingChar The percent-encoded string excluding characters
 *                           from the percent-encoded character
 *                           (alpha/num is included as default)
 */
PUBLIC GLOBAL AString TextParser::DoPercentEncodingEx(IN const AString& strData,
        IN const AString& strPeExcludingChar /*alpha/num is included as default*/)
{
    const IMS_SINT32 nLength = strData.GetLength();
    AStringBuffer objPeData(nLength * 2);
    AStringBuffer objHex(2);

    for (IMS_SINT32 i = 0; i < nLength; ++i)
    {
        IMS_CHAR c = strData[i];

        if (IMS_ISDIGIT(c))
        {
            objPeData.Append(c);
        }
        else if (IMS_ISALPHA(c))
        {
            objPeData.Append(c);
        }
        else if ((strPeExcludingChar.GetLength() > 0) && strPeExcludingChar.Contains(c))
        {
            objPeData.Append(c);
        }
        else
        {
            objHex.Sprintf("%02X", c);

            objPeData.Append(CHAR_PERCENT);
            objPeData.Append(objHex[0]);
            objPeData.Append(objHex[1]);
        }
    }

    return static_cast<const AStringBuffer&>(objPeData).GetString();
}

PUBLIC GLOBAL IMS_SINT32 TextParser::GetIndexOfDelimiter(IN const AString& strValue,
        IN const IMS_CHAR cDelimiter, IN IMS_BOOL bCheckDquot /*= IMS_TRUE*/)
{
    if (!bCheckDquot)
    {
        return strValue.GetIndexOf(cDelimiter);
    }
    else
    {
        IMS_SINT32 nDquot = 0;
        IMS_SINT32 nIndex = 0;
        IMS_CHAR cOld = 0;

        while (nIndex < strValue.GetLength())
        {
            const IMS_CHAR cNow = strValue[nIndex];

            if ((cNow == CHAR_DQUOT) && (nDquot == 0) && (cOld != '\\'))
            {
                nIndex++;
                nDquot++;
                cOld = cNow;
                continue;
            }

            if ((cNow == CHAR_DQUOT) && (nDquot != 0) && (cOld != '\\'))
            {
                nIndex++;
                nDquot--;
                cOld = cNow;
                continue;
            }

            if ((cNow == cDelimiter) && (nDquot == 0))
            {
                return nIndex;
            }

            nIndex++;

            if ((cOld == '\\') && (cNow == '\\'))
            {
                cOld = 0;
            }
            else
            {
                cOld = cNow;
            }
        }

        return AString::NPOS;
    }
}

PUBLIC GLOBAL IMS_BOOL TextParser::ParseMediaType(
        IN const AString& strMediaType, OUT AString& strType, OUT AString& strSubType)
{
    // Find a slash character ('/')
    IMS_SINT32 nSlashIndex = strMediaType.GetIndexOf(CHAR_SLASH);

    if (nSlashIndex == AString::NPOS)
    {
        return IMS_FALSE;
    }

    strType = strMediaType.GetSubStr(0, nSlashIndex);

    // Find a semicolon character (';') - start of parameters
    IMS_SINT32 nSemiColonIndex = strMediaType.GetIndexOf(CHAR_SEMICOLON, nSlashIndex + 1);

    if (nSemiColonIndex == AString::NPOS)
    {
        strSubType = strMediaType.GetSubStr(nSlashIndex + 1);
    }
    else
    {
        strSubType = strMediaType.GetSubStr(nSlashIndex + 1, nSemiColonIndex - (nSlashIndex + 1));
    }

    if ((strType.GetLength() == 0) || (strSubType.GetLength() == 0))
    {
        strType = AString::ConstNull();
        strSubType = AString::ConstNull();
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PUBLIC GLOBAL AString TextParser::TrimDquot(IN const AString& strValue)
{
    if (strValue.StartsWith(CHAR_DQUOT) && strValue.EndsWith(CHAR_DQUOT))
    {
        return strValue.GetSubStr(1, strValue.GetLength() - 2);
    }

    return strValue;
}

PRIVATE GLOBAL IMS_SINT32 TextParser::GetExpectedCountOfUtf8Cont(IN const IMS_CHAR c)
{
    if (c < 0xC0)
    {
        return 0;
    }
    else if (c <= 0xDF)
    {
        return 1;
    }
    else if (c <= 0xEF)
    {
        return 2;
    }
    else if (c <= 0xF7)
    {
        return 3;
    }
    else if (c <= 0xFB)
    {
        return 4;
    }
    else if (c <= 0xFD)
    {
        return 5;
    }

    return 0;
}

PRIVATE GLOBAL IMS_BOOL TextParser::IsHexCharacter(IN const IMS_CHAR c)
{
    if ((IMS_ISDIGIT(c)) || ((c >= 'A') && (c <= 'F')) || ((c >= 'a') && (c <= 'f')))
    {
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PRIVATE GLOBAL IMS_BOOL TextParser::IsParameterString(IN const AString& strParameter)
{
    IMS_SINT32 nIndex = strParameter.GetIndexOf(CHAR_EQUAL);

    // No parameter value (parameter name only)
    if (nIndex == AString::NPOS)
    {
        return IsTokenString(strParameter);
    }
    else
    {
        return (IsTokenString(strParameter.GetSubStr(0, nIndex)) &&
                IsParameterValueString(strParameter.GetSubStr(nIndex + 1)));
    }
}

PRIVATE GLOBAL IMS_BOOL TextParser::IsParameterValueString(IN const AString& strParameterValue)
{
    return (IsTokenString(strParameterValue) || IsQuotedString(strParameterValue));
}

// From RFC 3261
PRIVATE GLOBAL IMS_BOOL TextParser::IsQdText(IN const IMS_CHAR c)
{
    // HTAB, SP, %x21, %x23-5B, %x5D-7E,
    // UTF8-NONASCII <- It will be handled another method...
    if ((c == 0x09) || (c == 0x20) || (c == 0x21) || ((c >= 0x23) && (c <= 0x5B)) ||
            ((c >= 0x5D) && (c <= 0x7E)))
    {
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

// Compliant to RFC 2045
PRIVATE GLOBAL IMS_BOOL TextParser::IsQuotedString(IN const AString& strQuotedString)
{
    // ""
    if (strQuotedString.GetLength() < 2)
    {
        return IMS_FALSE;
    }

    if ((strQuotedString[0] != CHAR_DQUOT) ||
            (strQuotedString[strQuotedString.GetLength() - 1] != CHAR_DQUOT))
    {
        return IMS_FALSE;
    }

    IMS_SINT32 nEndPos = strQuotedString.GetLength() - 1;
    IMS_SINT32 nTrackPos = 1;

    while (nTrackPos < nEndPos)
    {
        const IMS_CHAR cChar = strQuotedString[nTrackPos];
        IMS_SINT32 nUtf8ContCount = GetExpectedCountOfUtf8Cont(cChar);

        // qdtext
        if (IsQdText(cChar))
        {
            // ok!!!
        }
        // UTF8-NONASCII
        else if (nUtf8ContCount > 0)
        {
            if (nEndPos <= (nTrackPos + nUtf8ContCount))
            {
                return IMS_FALSE;
            }

            // The current character is UTF8-NONASCII character.
            for (IMS_SINT32 i = 0; i < nUtf8ContCount; ++i)
            {
                ++nTrackPos;
                if (!IsUtf8Cont(strQuotedString[nTrackPos]))
                {
                    return IMS_FALSE;
                }
            }
        }
        // backslash('\')
        else if (cChar == CHAR_BACKSLASH)
        {
            ++nTrackPos;

            if (nEndPos <= nTrackPos)
            {
                return IMS_FALSE;
            }

            // TODO::????
            if ((strQuotedString[nTrackPos] != CHAR_BACKSLASH) &&
                    (strQuotedString[nTrackPos] != CHAR_DQUOT))
            {
                return IMS_FALSE;
            }
        }

        ++nTrackPos;
    }

    return IMS_TRUE;
}

// Compliant to RFC 3986
PRIVATE GLOBAL IMS_BOOL TextParser::IsReservedCharacter(IN const IMS_CHAR c)
{
    // "!" / "*" / "'" / "(" / ")" / ";" / ":" / "@" / "&" / "=" / "+" / "$" /
    // "," / "/" / "?" / "%" / "#" / "[" / "]"
    const IMS_CHAR RESERVED[] = "!*'();:@&=+$,/?%#[]";
    const IMS_SINT32 RESERVED_COUNT = 19;

    for (IMS_SINT32 i = 0; i < RESERVED_COUNT; ++i)
    {
        if (RESERVED[i] == c)
        {
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

PRIVATE GLOBAL IMS_BOOL TextParser::IsTokenString(IN const AString& strToken)
{
    if (strToken.GetLength() == 0)
    {
        return IMS_FALSE;
    }

    for (IMS_SINT32 i = 0; i < strToken.GetLength(); ++i)
    {
        if (!IsTokenCharacter(strToken[i]))
        {
            return IMS_FALSE;
        }
    }

    return IMS_TRUE;
}

// From RFC 3261
PRIVATE GLOBAL IMS_BOOL TextParser::IsUtf8Cont(IN const IMS_CHAR c)
{
    return ((c >= 0x80) && (c <= 0xBF));
}
