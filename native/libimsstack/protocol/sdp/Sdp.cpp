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
#include "AStringArray.h"
#include "ImsLib.h"

#include "Sdp.h"

PUBLIC GLOBAL const IMS_CHAR Sdp::STR_ADDR_TYPE_IP4[] = "IP4";
PUBLIC GLOBAL const IMS_CHAR Sdp::STR_ADDR_TYPE_IP6[] = "IP6";
PUBLIC GLOBAL const IMS_CHAR Sdp::STR_NET_TYPE_IN[] = "IN";
PUBLIC GLOBAL const IMS_CHAR* Sdp::STR_A_SETUP[Sdp::SETUP_MAX] = {
        "active",
        "passive",
        "actpass",
        "holdconn",
};

PUBLIC GLOBAL const IMS_CHAR* Sdp::STR_A_CONNECTION[Sdp::CONNECTION_MAX] = {
        "new",
        "existing",
};

PUBLIC GLOBAL IMS_BOOL Sdp::IsDigitString(IN const AString& strValue)
{
    if (strValue.GetLength() == 0)
    {
        return IMS_FALSE;
    }

    for (IMS_SINT32 i = 0; i < strValue.GetLength(); ++i)
    {
        if (!IMS_ISDIGIT(strValue[i]))
        {
            return IMS_FALSE;
        }
    }

    return IMS_TRUE;
}

PUBLIC GLOBAL IMS_BOOL Sdp::IsFqdnString(IN const AString& strValue)
{
    // FQDN = 4 *(alpha-numeric / "-" / ".")

    if (strValue.GetLength() < 4)
    {
        return IMS_FALSE;
    }

    for (IMS_SINT32 i = 0; i < strValue.GetLength(); ++i)
    {
        if (!IMS_ISALPHA(strValue[i]) && !IMS_ISDIGIT(strValue[i]) &&
                (strValue[i] != TextParser::CHAR_HYPHEN) && (strValue[i] != TextParser::CHAR_DOT))
        {
            return IMS_FALSE;
        }
    }

    return IMS_TRUE;
}

PUBLIC GLOBAL IMS_BOOL Sdp::IsTextString(IN const AString& strValue)
{
    if (strValue.GetLength() == 0)
    {
        return IMS_FALSE;
    }

    for (IMS_SINT32 i = 0; i < strValue.GetLength(); ++i)
    {
        if (!IsByteCharacter(strValue[i]))
        {
            return IMS_FALSE;
        }
    }

    return IMS_TRUE;
}

PUBLIC GLOBAL IMS_BOOL Sdp::IsTokenString(
        IN const AString& strValue, IN IMS_BOOL bAllowSpace /*= IMS_FALSE*/)
{
    if (strValue.GetLength() == 0)
    {
        return IMS_FALSE;
    }

    for (IMS_SINT32 i = 0; i < strValue.GetLength(); ++i)
    {
        if (!TextParser::IsTokenCharacter(strValue[i]))
        {
            if (bAllowSpace && (strValue[i] == 0x20))
            {
                continue;
            }

            return IMS_FALSE;
        }
    }

    return IMS_TRUE;
}

PUBLIC GLOBAL IMS_BOOL Sdp::IsTypedTimeString(IN const AString& strValue)
{
    if (strValue.GetLength() == 0)
    {
        return IMS_FALSE;
    }

    for (IMS_SINT32 i = 0; i < strValue.GetLength(); ++i)
    {
        // d, h, m, s
        if (!IMS_ISDIGIT(strValue[i]) && (strValue[i] != 0x64) && (strValue[i] != 0x68) &&
                (strValue[i] != 0x6D) && (strValue[i] != 0x73))
        {
            return IMS_FALSE;
        }
    }

    return IMS_TRUE;
}

PUBLIC GLOBAL IMS_BOOL Sdp::IsNonWsString(IN const AString& strValue)
{
    if (strValue.GetLength() == 0)
    {
        return IMS_FALSE;
    }

    for (IMS_SINT32 i = 0; i < strValue.GetLength(); ++i)
    {
        if (!IsVisibleCharacter(strValue[i]))
        {
            return IMS_FALSE;
        }
    }

    return IMS_TRUE;
}

PUBLIC GLOBAL IMS_BOOL Sdp::IsUriString(IN const AString& strValue)
{
    if (strValue.GetLength() == 0)
    {
        return IMS_FALSE;
    }

    // TODO: validation check

    return IMS_TRUE;
}

PUBLIC GLOBAL IMS_BOOL Sdp::SplitLine(
        IN const AString& strValue, IN IMS_SINT32 nNumOfParts, OUT AStringArray& objTokens)
{
    if (nNumOfParts <= 1)
    {
        // No need to split the specified value and returns the original value.
        objTokens.AddElement(strValue);
        return IMS_TRUE;
    }

    IMS_SINT32 nStartOffset = 0;

    for (IMS_SINT32 i = 0; i < nNumOfParts - 1; ++i)
    {
        IMS_SINT32 nEndOffset = strValue.GetIndexOf(TextParser::CHAR_SP, nStartOffset);

        if (nEndOffset == AString::NPOS)
        {
            // Invalid SDP line format
            return IMS_FALSE;
        }

        objTokens.AddElement(strValue.GetSubStr(nStartOffset, nEndOffset - nStartOffset));

        nStartOffset = nEndOffset + 1;
    }

    objTokens.AddElement(strValue.GetSubStr(nStartOffset));

    return IMS_TRUE;
}

PUBLIC GLOBAL IMS_UINT32 Sdp::ConvertTypedTimeToSeconds(IN const AString& strValue)
{
    IMS_UINT32 nMultiplier = 1;
    AString strDigit;

    if (strValue.GetLength() == 0)
    {
        return 0;
    }

    if (strValue.EndsWith('d'))
    {
        nMultiplier = 86400;
        strDigit = strValue.GetSubStr(0, strValue.GetLength() - 1);
    }
    else if (strValue.EndsWith('h'))
    {
        nMultiplier = 3600;
        strDigit = strValue.GetSubStr(0, strValue.GetLength() - 1);
    }
    else if (strValue.EndsWith('m'))
    {
        nMultiplier = 60;
        strDigit = strValue.GetSubStr(0, strValue.GetLength() - 1);
    }
    else if (strValue.EndsWith('s'))
    {
        nMultiplier = 1;
        strDigit = strValue.GetSubStr(0, strValue.GetLength() - 1);
    }
    else
    {
        nMultiplier = 1;
        strDigit = strValue;
    }

    return IMS_UINT32(strDigit.ToUInt32() * nMultiplier);
}

PUBLIC GLOBAL IMS_SINT32 Sdp::GetPayloadTypeFromAttribute(IN const AString& strValue)
{
    IMS_SINT32 nSpIndex = strValue.GetIndexOf(TextParser::CHAR_SP);

    if (nSpIndex == AString::NPOS)
    {
        return (-1);
    }

    AString strPayloadType = strValue.GetSubStr(0, nSpIndex);
    IMS_BOOL bOk = IMS_FALSE;
    IMS_SINT32 nPayloadType = strPayloadType.ToInt32(&bOk);

    if (!bOk)
    {
        return (-1);
    }

    return nPayloadType;
}

PUBLIC GLOBAL AString Sdp::IncreaseSessionVersion(IN const AString& strValue)
{
    if (strValue.GetLength() == 0)
    {
        return strValue;
    }

    AString strNewVersion(strValue);

    for (IMS_SINT32 i = strNewVersion.GetLength() - 1; i >= 0; --i)
    {
        AString::CharRef objChar = strNewVersion[i];

        if (objChar == '9')
        {
            // Character replacement.
            // cppcheck-suppress unreadVariable
            objChar = '0';
        }
        else
        {
            const IMS_CHAR ch = objChar;
            // Character replacement.
            // cppcheck-suppress unreadVariable
            objChar = ch + 1;
            break;
        }
    }

    if (strNewVersion[0] == '0')
    {
        strNewVersion.Prepend('1');
    }

    return strNewVersion;
}

PUBLIC GLOBAL IMS_BOOL Sdp::ParseAttributeRtpmap(IN const AString& strValue,
        OUT IMS_SINT32& nPayloadType, OUT AString& strEncodingName, OUT IMS_UINT32& nClockRate,
        OUT AString& strEncodingParameters)
{
    ImsList<AString> objTokens = strValue.Split(TextParser::CHAR_SP);

    if (objTokens.GetSize() < 2)
    {
        return IMS_FALSE;
    }

    // Payload type
    nPayloadType = objTokens.GetAt(0).ToInt32();

    ImsList<AString> objTokens2 = objTokens.GetAt(1).Split(TextParser::CHAR_SLASH);

    if (objTokens2.GetSize() < 2)
    {
        return IMS_FALSE;
    }

    // Encoding name
    strEncodingName = objTokens2.GetAt(0);

    // Clock rate
    nClockRate = objTokens2.GetAt(1).ToUInt32();

    if (objTokens2.GetSize() > 2)
    {
        // Encoding parameters
        strEncodingParameters = objTokens2.GetAt(2);
    }

    return IMS_TRUE;
}

PUBLIC GLOBAL IMS_BOOL Sdp::ParseAttributeFmtp(
        IN const AString& strValue, OUT IMS_SINT32& nPayloadType, OUT AString& strParameters)
{
    IMS_SINT32 nSpIndex = strValue.GetIndexOf(TextParser::CHAR_SP);
    AString strPayloadType = strValue.GetSubStr(0, nSpIndex);
    IMS_BOOL bOk = IMS_FALSE;

    // Payload type
    nPayloadType = strPayloadType.ToInt32(&bOk);

    if (!bOk)
    {
        return IMS_FALSE;
    }

    // Format specific parameters
    if (nSpIndex != AString::NPOS)
    {
        strParameters = strValue.GetSubStr(nSpIndex + 1);

        if (strParameters.GetLength() == 0)
        {
            strParameters = AString::ConstEmpty();
        }
    }

    return IMS_TRUE;
}

PUBLIC GLOBAL IMS_BOOL Sdp::ParseAttributeRtcp(IN const AString& strValue, OUT IMS_SINT32& nPort)
{
    // a=rtcp:<port>SP<nettype>SP<addrtype>SP<connection-address>
    if (strValue.GetLength() == 0)
    {
        return IMS_FALSE;
    }

    ImsList<AString> objTokens = strValue.Split(TextParser::CHAR_SP);

    if (objTokens.GetSize() < 1)
    {
        return IMS_FALSE;
    }

    // Port number
    IMS_BOOL bOk = IMS_FALSE;
    IMS_SINT32 nRtcpPort = objTokens.GetAt(0).ToInt32(&bOk);

    if (!bOk)
    {
        return IMS_FALSE;
    }

    nPort = nRtcpPort;

    // nettype, addrtype, connection-address is not parsed in this moment

    return IMS_TRUE;
}

PUBLIC GLOBAL void Sdp::ParseAttributeSetup(
        IN const AString& strValue, OUT IMS_SINT32& nTypeOfSetup)
{
    // a=setup:<type>

    for (IMS_SINT32 i = 0; i < SETUP_MAX; ++i)
    {
        if (strValue.EqualsIgnoreCase(STR_A_SETUP[i]))
        {
            nTypeOfSetup = i;
            return;
        }
    }

    nTypeOfSetup = SETUP_NONE;
}

PUBLIC GLOBAL void Sdp::ParseAttributeConnection(
        IN const AString& strValue, OUT IMS_SINT32& nTypeOfConnection)
{
    // a=connection:<type>

    for (IMS_SINT32 i = 0; i < CONNECTION_MAX; ++i)
    {
        if (strValue.EqualsIgnoreCase(STR_A_CONNECTION[i]))
        {
            nTypeOfConnection = i;
            return;
        }
    }

    nTypeOfConnection = CONNECTION_NONE;
}

PUBLIC GLOBAL IMS_BOOL Sdp::ParseAttributeFramesize(IN const AString& strValue,
        OUT IMS_SINT32& nPayloadType, OUT IMS_SINT32& nWidth, OUT IMS_SINT32& nHeight)
{
    // a=framesize:<payload type>SP<width>-<height>

    IMS_SINT32 nSpIndex = strValue.GetIndexOf(TextParser::CHAR_SP);

    nPayloadType = (-1);
    nWidth = 0;
    nHeight = 0;

    if (nSpIndex == AString::NPOS)
    {
        return IMS_FALSE;
    }

    IMS_BOOL bOk = IMS_FALSE;
    AString strTmp = strValue.GetSubStr(0, nSpIndex);

    nPayloadType = strTmp.ToInt32(&bOk);

    if (!bOk)
    {
        nPayloadType = (-1);
        return IMS_FALSE;
    }

    IMS_SINT32 nHyphenIndex = strValue.GetIndexOf(TextParser::CHAR_HYPHEN, nSpIndex + 1);

    if (nHyphenIndex == AString::NPOS)
    {
        nPayloadType = (-1);
        return IMS_FALSE;
    }

    // <width>
    strTmp = strValue.GetSubStr(nSpIndex + 1, nHyphenIndex - nSpIndex - 1);
    nWidth = strTmp.ToInt32(&bOk);

    if (!bOk)
    {
        nPayloadType = (-1);
        return IMS_FALSE;
    }

    // <height>
    strTmp = strValue.GetSubStr(nHyphenIndex + 1);
    nHeight = strTmp.ToInt32(&bOk);

    if (!bOk)
    {
        nPayloadType = (-1);
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PRIVATE GLOBAL IMS_BOOL Sdp::IsByteCharacter(IN const IMS_CHAR c)
{
    // Any byte except NUL, CR or LF

    if ((c == 0x00) || (c == 0x0A) || (c == 0x0D))
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PRIVATE GLOBAL IMS_BOOL Sdp::IsVisibleCharacter(IN const IMS_CHAR c)
{
    // 0x21 ~ 0x7E, 0x80 ~ 0xFF
    IMS_SINT32 nValue = c;

    if ((nValue >= 0x21) && (nValue <= 0x7E))
    {
        return IMS_TRUE;
    }

    if ((nValue >= 0x80) && (nValue <= 0xFF))
    {
        return IMS_TRUE;
    }

    return IMS_FALSE;
}
