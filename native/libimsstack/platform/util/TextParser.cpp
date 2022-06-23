/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090904  toastops@                 Created
    </table>

    Description
     This class is the class for a global enumerations, definitions and constant variables.
*/

#include "ServiceMemory.h"
#include "ImsLib.h"
#include "AStringBuffer.h"
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

/*

Remarks

*/
PUBLIC GLOBAL const IMS_CHAR* TextParser::BooleanToString(
        IN IMS_BOOL bValue, IN IMS_BOOL bLowerCase /* = IMS_TRUE */)
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

/*

Remarks

*/
PUBLIC GLOBAL AString TextParser::CharToHexString(IN CONST IMS_CHAR cChar)
{
    AString strHex;

    return strHex.Sprintf("%02X", cChar);
}

/*

Remarks

*/
PUBLIC GLOBAL IMS_SINT32 TextParser::HexStringToChar(IN CONST AString& strHex)
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

    IMS_CHAR ch;
    const IMS_CHAR chFirst = strTmpHex[0];
    const IMS_CHAR chSecond = strTmpHex[1];

    if ((chFirst >= 'A') && (chFirst <= 'F'))
    {
        ch = chFirst - 'A' + 10;
    }
    else if ((chFirst >= 'a') && (chFirst <= 'f'))
    {
        ch = chFirst - 'a' + 10;
    }
    else
    {
        ch = chFirst - '0';
    }

    ch *= 16;

    if ((chSecond >= 'A') && (chSecond <= 'F'))
    {
        ch += chSecond - 'A' + 10;
    }
    else if ((chSecond >= 'a') && (chSecond <= 'f'))
    {
        ch += chSecond - 'a' + 10;
    }
    else
    {
        ch += chSecond - '0';
    }

    return ch;
}

/*

Remarks
 Compliant to RFC 2045

*/
PUBLIC GLOBAL IMS_BOOL TextParser::IsTokenCharacter(IN CONST IMS_CHAR ch)
{
    if (ch == 0x21)
    {
        return IMS_TRUE;
    }

    if ((ch >= 0x23) && (ch <= 0x27))
    {
        return IMS_TRUE;
    }

    if ((ch >= 0x2A) && (ch <= 0x2B))
    {
        return IMS_TRUE;
    }

    if ((ch >= 0x2D) && (ch <= 0x2E))
    {
        return IMS_TRUE;
    }

    if ((ch >= 0x30) && (ch <= 0x39))
    {
        return IMS_TRUE;
    }

    if ((ch >= 0x41) && (ch <= 0x5A))
    {
        return IMS_TRUE;
    }

    if ((ch >= 0x5E) && (ch <= 0x7E))
    {
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

/*
 This method validates the MIME media types (in general, Content-Type header)
according to RFC 2045 "MIME Part One: Format of Internet Message Bodies"

Remarks

*/
PUBLIC GLOBAL IMS_BOOL TextParser::IsValidMediaType(IN CONST AString& strMediaType)
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
    IMSList<AString> objParameters =
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

/*
 Converts a percent-encoded URI string to an original URI string.

Remarks

*/
PUBLIC GLOBAL AString TextParser::DoPercentDecoding(IN CONST AString& strPEData)
{
    if (!strPEData.Contains(CHAR_PERCENT))
    {
        return strPEData;
    }

    const IMS_SINT32 nLength = strPEData.GetLength();
    AStringBuffer objBuffer(nLength);

    for (IMS_SINT32 i = 0; i < nLength; ++i)
    {
        if (strPEData[i] != CHAR_PERCENT)
        {
            objBuffer.Append(strPEData[i]);
        }
        else
        {
            AString strHexDigs = strPEData.GetSubStr(i + 1, 2);
            IMS_SINT32 ch = HexStringToChar(strHexDigs);

            if ((ch < 0x00) || (ch > 0xFF))
            {
                objBuffer.Append(CHAR_PERCENT);
                objBuffer.Append(strHexDigs);
            }
            else
            {
                objBuffer.Append(static_cast<const IMS_CHAR>(ch));
            }

            i += 2;
        }
    }

    return static_cast<const AStringBuffer&>(objBuffer).GetString();
}

/*
 Converts an original URI string to a percent-encoded URI string.

Remarks
    strPEExtraChar - including characters from the non-reserved characters
    strPEExcludingChar - excluding characters from the reserved characters

*/
PUBLIC GLOBAL AString TextParser::DoPercentEncoding(IN CONST AString& strData,
        IN CONST AString& strPEExtraChar /* = AString::ConstNull() */,
        IN CONST AString& strPEExcludingChar /* = AString::ConstNull() */)
{
    const IMS_SINT32 nLength = strData.GetLength();
    AStringBuffer objPEData(nLength * 2);
    AStringBuffer objHEX(2);

    for (IMS_SINT32 i = 0; i < nLength; ++i)
    {
        if (!IsReservedCharacter(strData[i]))
        {
            if (!strPEExtraChar.IsNULL() && strPEExtraChar.Contains(strData[i]))
            {
                objHEX.Sprintf("%02X", strData[i]);

                objPEData.Append(CHAR_PERCENT);
                objPEData.Append(objHEX[0]);
                objPEData.Append(objHEX[1]);
            }
            else
            {
                objPEData.Append(strData[i]);
            }
        }
        else
        {
            if (!strPEExcludingChar.IsNULL() && strPEExcludingChar.Contains(strData[i]))
            {
                objPEData.Append(strData[i]);
            }
            else
            {
                objHEX.Sprintf("%02X", strData[i]);

                objPEData.Append(CHAR_PERCENT);
                objPEData.Append(objHEX[0]);
                objPEData.Append(objHEX[1]);
            }
        }
    }

    return static_cast<const AStringBuffer&>(objPEData).GetString();
}

/*
 Converts an original URI string to a percent-encoded URI string.

Remarks
    strPEExcludingChar - excluding characters from the percent-encoded character
    (alpha/num is included as default)

*/
PUBLIC GLOBAL AString TextParser::DoPercentEncodingEx(IN CONST AString& strData,
        IN CONST AString& strPEExcludingChar /* alpha/num is included as default */)
{
    const IMS_SINT32 nLength = strData.GetLength();
    AStringBuffer objPEData(nLength * 2);
    AStringBuffer objHEX(2);

    for (IMS_SINT32 i = 0; i < nLength; ++i)
    {
        IMS_CHAR ch = strData[i];

        if (IMS_ISDIGIT(ch))
        {
            objPEData.Append(ch);
        }
        else if (IMS_ISALPHA(ch))
        {
            objPEData.Append(ch);
        }
        else if ((strPEExcludingChar.GetLength() > 0) && strPEExcludingChar.Contains(ch))
        {
            objPEData.Append(ch);
        }
        else
        {
            objHEX.Sprintf("%02X", ch);

            objPEData.Append(CHAR_PERCENT);
            objPEData.Append(objHEX[0]);
            objPEData.Append(objHEX[1]);
        }
    }

    return static_cast<const AStringBuffer&>(objPEData).GetString();
}

/*

Remarks

*/
PUBLIC GLOBAL IMS_SINT32 TextParser::GetIndexOfDelimiter(IN CONST AString& strValue,
        IN CONST IMS_CHAR cDelimiter, IN IMS_BOOL bCheckDQUOT /*= IMS_TRUE */)
{
    if (!bCheckDQUOT)
    {
        return strValue.GetIndexOf(cDelimiter);
    }
    else
    {
        IMS_SINT32 nDQUOT = 0;
        IMS_SINT32 nIndex = 0;
        IMS_CHAR cOld = 0;

        while (nIndex < strValue.GetLength())
        {
            const IMS_CHAR cNow = strValue[nIndex];

            if ((cNow == CHAR_DQUOT) && (nDQUOT == 0) && (cOld != '\\'))
            {
                nIndex++;
                nDQUOT++;
                cOld = cNow;
                continue;
            }

            if ((cNow == CHAR_DQUOT) && (nDQUOT != 0) && (cOld != '\\'))
            {
                nIndex++;
                nDQUOT--;
                cOld = cNow;
                continue;
            }

            if ((cNow == cDelimiter) && (nDQUOT == 0))
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

/*

Remarks

*/
PUBLIC GLOBAL IMS_BOOL TextParser::ParseMediaType(
        IN CONST AString& strMediaType, OUT AString& strType, OUT AString& strSubType)
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

/*

Remarks

*/
PUBLIC GLOBAL AString TextParser::TrimDQUOT(IN CONST AString& strValue)
{
    if (strValue.StartsWith(CHAR_DQUOT) && strValue.EndsWith(CHAR_DQUOT))
    {
        return strValue.GetSubStr(1, strValue.GetLength() - 2);
    }

    return strValue;
}

/*

Remarks

*/
PRIVATE GLOBAL IMS_SINT32 TextParser::GetExpectedCountOfUTF8Cont(IN CONST IMS_CHAR ch)
{
    if (ch < 0xC0)
    {
        return 0;
    }
    else if (ch <= 0xDF)
    {
        return 1;
    }
    else if (ch <= 0xEF)
    {
        return 2;
    }
    else if (ch <= 0xF7)
    {
        return 3;
    }
    else if (ch <= 0xFB)
    {
        return 4;
    }
    else if (ch <= 0xFD)
    {
        return 5;
    }

    return 0;
}

/*

Remarks

*/
PRIVATE GLOBAL IMS_BOOL TextParser::IsHexCharacter(IN CONST IMS_CHAR ch)
{
    if ((IMS_ISDIGIT(ch)) || ((ch >= 'A') && (ch <= 'F')) || ((ch >= 'a') && (ch <= 'f')))
    {
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

/*

Remarks

*/
PRIVATE GLOBAL IMS_BOOL TextParser::IsParameterString(IN CONST AString& strParameter)
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

/*

Remarks

*/
PRIVATE GLOBAL IMS_BOOL TextParser::IsParameterValueString(IN CONST AString& strParameterValue)
{
    return (IsTokenString(strParameterValue) || IsQuotedString(strParameterValue));
}

/*

Remarks
 From RFC 3261

*/
PRIVATE GLOBAL IMS_BOOL TextParser::IsQdText(IN CONST IMS_CHAR ch)
{
    // HTAB, SP, %x21, %x23-5B, %x5D-7E,
    // UTF8-NONASCII <- It will be handled another method...
    if ((ch == 0x09) || (ch == 0x20) || (ch == 0x21) || ((ch >= 0x23) && (ch <= 0x5B)) ||
            ((ch >= 0x5D) && (ch <= 0x7E)))
    {
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

/*

Remarks
 Compliant to RFC 2045

*/
PRIVATE GLOBAL IMS_BOOL TextParser::IsQuotedString(IN CONST AString& strQuotedString)
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
        IMS_SINT32 nUTF8ContCount = GetExpectedCountOfUTF8Cont(cChar);

        // qdtext
        if (IsQdText(cChar))
        {
            // ok!!!
        }
        // UTF8-NONASCII
        else if (nUTF8ContCount > 0)
        {
            if (nEndPos <= (nTrackPos + nUTF8ContCount))
            {
                return IMS_FALSE;
            }

            // The current character is UTF8-NONASCII character.
            for (IMS_SINT32 i = 0; i < nUTF8ContCount; ++i)
            {
                ++nTrackPos;
                if (!IsUTF8Cont(strQuotedString[nTrackPos]))
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

/*

Remarks
 Compliant to RFC 3986

*/
PRIVATE GLOBAL IMS_BOOL TextParser::IsReservedCharacter(IN CONST IMS_CHAR ch)
{
    // "!" / "*" / "'" / "(" / ")" / ";" / ":" / "@" / "&" / "=" / "+" / "$" /
    // "," / "/" / "?" / "%" / "#" / "[" / "]"
    const IMS_CHAR RESERVED[] = "!*'();:@&=+$,/?%#[]";
    const IMS_SINT32 RESERVED_COUNT = 19;

    for (IMS_SINT32 i = 0; i < RESERVED_COUNT; ++i)
    {
        if (RESERVED[i] == ch)
        {
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

/*

Remarks

*/
PRIVATE GLOBAL IMS_BOOL TextParser::IsTokenString(IN CONST AString& strToken)
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

/*

Remarks
 From RFC 3261

*/
PRIVATE GLOBAL IMS_BOOL TextParser::IsUTF8Cont(IN CONST IMS_CHAR ch)
{
    return ((ch >= 0x80) && (ch <= 0xBF));
}
