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
#include "TextParser.h"

#include "Sdp.h"
#include "SdpBandwidth.h"

PUBLIC GLOBAL const IMS_CHAR SdpBandwidth::TOKEN_AS[] = "AS";
PUBLIC GLOBAL const IMS_CHAR SdpBandwidth::TOKEN_CT[] = "CT";
PUBLIC GLOBAL const IMS_CHAR SdpBandwidth::TOKEN_RR[] = "RR";
PUBLIC GLOBAL const IMS_CHAR SdpBandwidth::TOKEN_RS[] = "RS";
PUBLIC GLOBAL const IMS_CHAR SdpBandwidth::TOKEN_TIAS[] = "TIAS";

PUBLIC
SdpBandwidth::SdpBandwidth() :
        SdpLine(),
        m_nType(TYPE_OTHER),
        m_strType(AString::ConstNull()),
        m_nBandwidth(INVALID_BANDWIDTH)
{
}

PUBLIC
SdpBandwidth::SdpBandwidth(IN const SdpBandwidth& other) :
        SdpLine(other),
        m_nType(other.m_nType),
        m_strType(other.m_strType),
        m_nBandwidth(other.m_nBandwidth)
{
}

PUBLIC VIRTUAL SdpBandwidth::~SdpBandwidth() {}

PUBLIC
SdpBandwidth& SdpBandwidth::operator=(IN const SdpBandwidth& other)
{
    if (this != &other)
    {
        SdpLine::operator=(other);

        m_nType = other.m_nType;
        m_strType = other.m_strType;
        m_nBandwidth = other.m_nBandwidth;
    }

    return (*this);
}

PUBLIC VIRTUAL IMS_BOOL SdpBandwidth::Decode(IN const AString& strValue)
{
    // b=<bwtype>:<bandwidth>
    ImsList<AString> objTokens = strValue.Split(TextParser::CHAR_COLON);

    if (objTokens.GetSize() != 2)
    {
        // Invalid bandwidth line
        return IMS_FALSE;
    }

    // Check the validity
    if (!Sdp::IsTokenString(objTokens.GetAt(0)))
    {
        // Invalid bwtype field
        return IMS_FALSE;
    }

    if ((objTokens.GetAt(1).GetLength() == 0) || !Sdp::IsDigitString(objTokens.GetAt(1)))
    {
        // Invalid bandwidth field
        return IMS_FALSE;
    }

    // bwtype field
    m_strType = objTokens.GetAt(0);

    if (m_strType.Equals(TOKEN_AS))
    {
        m_nType = TYPE_AS;
    }
    else if (m_strType.Equals(TOKEN_CT))
    {
        m_nType = TYPE_CT;
    }
    else if (m_strType.Equals(TOKEN_RR))
    {
        m_nType = TYPE_RR;
    }
    else if (m_strType.Equals(TOKEN_RS))
    {
        m_nType = TYPE_RS;
    }
    else if (m_strType.Equals(TOKEN_TIAS))
    {
        m_nType = TYPE_TIAS;
    }
    else
    {
        m_nType = TYPE_OTHER;
    }

    // bandwidth field
    m_nBandwidth = objTokens.GetAt(1).ToInt32();

    return IMS_TRUE;
}

PUBLIC VIRTUAL AString SdpBandwidth::Encode() const
{
    if (m_strType.GetLength() == 0)
    {
        return AString::ConstNull();
    }

    // b=<bwtype>:<bandwidth>
    AString strLine(1, Sdp::LINE_B);

    strLine.Append(TextParser::CHAR_EQUAL);
    strLine.Append(GetValue());
    strLine.Append(TextParser::CHAR_CR);
    strLine.Append(TextParser::CHAR_LF);

    return strLine;
}

PUBLIC VIRTUAL AString SdpBandwidth::GetValue() const
{
    AString strValue;

    // bwtype field
    strValue.Append(m_strType);
    strValue.Append(TextParser::STR_COLON);

    // bandwidth
    AString strTmp;
    strTmp.SetNumber(m_nBandwidth);

    strValue.Append(strTmp);

    return strValue;
}

PUBLIC
IMS_BOOL SdpBandwidth::Equals(IN const SdpBandwidth* pBandwidth) const
{
    if (pBandwidth == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (m_nType != pBandwidth->m_nType)
    {
        return IMS_FALSE;
    }

    if ((m_nType == TYPE_OTHER) && !m_strType.Equals(pBandwidth->m_strType))
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL SdpBandwidth::SetValue(IN IMS_SINT32 nType, IN IMS_SINT32 nBandwidth,
        IN const AString& strType /*= AString::ConstNull()*/)
{
    if (nBandwidth <= INVALID_BANDWIDTH)
    {
        return IMS_FALSE;
    }

    m_nType = nType;

    switch (m_nType)
    {
        case TYPE_AS:
            m_strType = TOKEN_AS;
            break;
        case TYPE_CT:
            m_strType = TOKEN_CT;
            break;
        case TYPE_RR:
            m_strType = TOKEN_RR;
            break;
        case TYPE_RS:
            m_strType = TOKEN_RS;
            break;
        case TYPE_TIAS:
            m_strType = TOKEN_TIAS;
            break;
        case TYPE_OTHER:
            if (!Sdp::IsTokenString(strType))
            {
                // Invalid bwtype field
                return IMS_FALSE;
            }

            m_strType = strType;
            break;
        default:
            return IMS_FALSE;
    }

    // bandwidth field
    m_nBandwidth = nBandwidth;

    return IMS_TRUE;
}
