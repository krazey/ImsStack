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
#include "SdpAttribute.h"

PRIVATE GLOBAL const IMS_CHAR* SdpAttribute::ATTRIBUTE[SdpAttribute::ATTRIBUTE_MAX] = {
        "cat",
        "keywds",
        "tool",
        "ptime",
        "maxptime",
        "rtpmap",
        "recvonly",
        "sendrecv",
        "sendonly",
        "inactive",
        "orient",
        "type",
        "charset",
        "sdplang",
        "lang",
        "framerate",
        "quality",
        "fmtp",
        "curr",
        "des",
        "conf",
        "mid",
        "group",
        "rtcp",
        "rtcp-xr",
        "maxprate",
        "setup",
        "connection",
        "label",
        "rtcp-fb",
        "content",
        "accept-types",
        "accept-wrapped-types",
        "max-size",
        "path",
        "candidate",
        "file-selector",
        "file-transfer-id",
        "file-disposition",
        "file-date",
        "file-icon",
        "file-range",
        "csup",
        "creq",
        "acap",
        "tcap",
        "pcfg",
        "acfg",
        "framesize",
        "imageattr",
        "crypto",
        "3ge2ae",
        "anbr",
        IMS_NULL,
};

PUBLIC
SdpAttribute::SdpAttribute() :
        SdpLine(),
        m_nFormat(FORMAT_PROPERTY_ATTRIBUTE),
        m_nAttribute(ATTRIBUTE_INVALID),
        m_strAttribute(AString::ConstNull()),
        m_strAttrValue(AString::ConstEmpty())
{
}

PUBLIC
SdpAttribute::SdpAttribute(IN const SdpAttribute& other) :
        SdpLine(other),
        m_nFormat(other.m_nFormat),
        m_nAttribute(other.m_nAttribute),
        m_strAttribute(other.m_strAttribute),
        m_strAttrValue(other.m_strAttrValue)
{
}

PUBLIC VIRTUAL SdpAttribute::~SdpAttribute() {}

PUBLIC
SdpAttribute& SdpAttribute::operator=(IN const SdpAttribute& other)
{
    if (this != &other)
    {
        SdpLine::operator=(other);

        m_nFormat = other.m_nFormat;
        m_nAttribute = other.m_nAttribute;
        m_strAttribute = other.m_strAttribute;
        m_strAttrValue = other.m_strAttrValue;
    }

    return (*this);
}

PUBLIC VIRTUAL IMS_BOOL SdpAttribute::Decode(IN const AString& strValue)
{
    // a=<attribute>
    // a=<attribute>:<value>
    IMS_SINT32 nColon = strValue.GetIndexOf(TextParser::CHAR_COLON);

    m_nAttribute = ATTRIBUTE_OTHER;

    if (nColon == AString::NPOS)
    {
        m_nFormat = FORMAT_PROPERTY_ATTRIBUTE;
        m_strAttribute = strValue.GetSubStr(0, nColon);
        m_strAttrValue = AString::ConstEmpty();
    }
    else
    {
        m_nFormat = FORMAT_VALUE_ATTRIBUTE;
        m_strAttribute = strValue.GetSubStr(0, nColon);
        m_strAttrValue = strValue.GetSubStr(nColon + 1);
    }

    if (!Sdp::IsTokenString(m_strAttribute))
    {
        // Invalid attribute field
        return IMS_FALSE;
    }

    // attribute field
    switch (m_strAttribute[0])
    {
        case '3':
            if (m_strAttribute.Equals(ATTRIBUTE[A_3GE2AE]))
            {
                m_nAttribute = A_3GE2AE;
            }
            break;

        case 'a':
            if (m_strAttribute.Equals(ATTRIBUTE[ACCEPT_TYPES]))
            {
                m_nAttribute = ACCEPT_TYPES;
            }
            else if (m_strAttribute.Equals(ATTRIBUTE[ACCEPT_WRAPPED_TYPES]))
            {
                m_nAttribute = ACCEPT_WRAPPED_TYPES;
            }
            else if (m_strAttribute.Equals(ATTRIBUTE[ACAP]))
            {
                m_nAttribute = ACAP;
            }
            else if (m_strAttribute.Equals(ATTRIBUTE[ACFG]))
            {
                m_nAttribute = ACFG;
            }
            else if (m_strAttribute.Equals(ATTRIBUTE[ANBR]))
            {
                m_nAttribute = ANBR;
            }
            break;

        case 'c':
            if (m_strAttribute.Equals(ATTRIBUTE[CAT]))
            {
                m_nAttribute = CAT;
            }
            else if (m_strAttribute.Equals(ATTRIBUTE[CHARSET]))
            {
                m_nAttribute = CHARSET;
            }
            else if (m_strAttribute.Equals(ATTRIBUTE[CONF]))
            {
                m_nAttribute = CONF;
            }
            else if (m_strAttribute.Equals(ATTRIBUTE[CONTENT]))
            {
                m_nAttribute = CONTENT;
            }
            else if (m_strAttribute.Equals(ATTRIBUTE[CURR]))
            {
                m_nAttribute = CURR;
            }
            else if (m_strAttribute.Equals(ATTRIBUTE[CONNECTION]))
            {
                m_nAttribute = CONNECTION;
            }
            else if (m_strAttribute.Equals(ATTRIBUTE[CANDIDATE]))
            {
                m_nAttribute = CANDIDATE;
            }
            else if (m_strAttribute.Equals(ATTRIBUTE[CSUP]))
            {
                m_nAttribute = CSUP;
            }
            else if (m_strAttribute.Equals(ATTRIBUTE[CREQ]))
            {
                m_nAttribute = CREQ;
            }
            else if (m_strAttribute.Equals(ATTRIBUTE[CRYPTO]))
            {
                m_nAttribute = CRYPTO;
            }
            break;

        case 'd':
            if (m_strAttribute.Equals(ATTRIBUTE[DES]))
            {
                m_nAttribute = DES;
            }
            break;

        case 'f':
            if (m_strAttribute.Equals(ATTRIBUTE[FMTP]))
            {
                m_nAttribute = FMTP;
            }
            else if (m_strAttribute.Equals(ATTRIBUTE[FRAMERATE]))
            {
                // <integer>.<fraction>
                m_nAttribute = FRAMERATE;
            }
            else if (m_strAttribute.Equals(ATTRIBUTE[FILE_SELECTOR]))
            {
                m_nAttribute = FILE_SELECTOR;
            }
            else if (m_strAttribute.Equals(ATTRIBUTE[FILE_TRANSFER_ID]))
            {
                m_nAttribute = FILE_TRANSFER_ID;
            }
            else if (m_strAttribute.Equals(ATTRIBUTE[FILE_DISPOSITION]))
            {
                m_nAttribute = FILE_DISPOSITION;
            }
            else if (m_strAttribute.Equals(ATTRIBUTE[FILE_DATE]))
            {
                m_nAttribute = FILE_DATE;
            }
            else if (m_strAttribute.Equals(ATTRIBUTE[FILE_ICON]))
            {
                m_nAttribute = FILE_ICON;
            }
            else if (m_strAttribute.Equals(ATTRIBUTE[FILE_RANGE]))
            {
                m_nAttribute = FILE_RANGE;
            }
            else if (m_strAttribute.Equals(ATTRIBUTE[FRAMESIZE]))
            {
                m_nAttribute = FRAMESIZE;
            }
            break;

        case 'g':
            if (m_strAttribute.Equals(ATTRIBUTE[GROUP]))
            {
                m_nAttribute = GROUP;
            }
            break;

        case 'i':
            if (m_strAttribute.Equals(ATTRIBUTE[INACTIVE]))
            {
                m_nAttribute = INACTIVE;
            }
            else if (m_strAttribute.Equals(ATTRIBUTE[IMAGEATTR]))
            {
                m_nAttribute = IMAGEATTR;
            }
            break;

        case 'k':
            if (m_strAttribute.Equals(ATTRIBUTE[KEYWDS]))
            {
                m_nAttribute = KEYWDS;
            }
            break;

        case 'l':
            if (m_strAttribute.Equals(ATTRIBUTE[LABEL]))
            {
                m_nAttribute = LABEL;
            }
            else if (m_strAttribute.Equals(ATTRIBUTE[LANG]))
            {
                m_nAttribute = LANG;
            }
            break;

        case 'm':
            if (m_strAttribute.Equals(ATTRIBUTE[MAX_PRATE]))
            {
                m_nAttribute = MAX_PRATE;
            }
            else if (m_strAttribute.Equals(ATTRIBUTE[MAXPTIME]))
            {
                m_nAttribute = MAXPTIME;
            }
            else if (m_strAttribute.Equals(ATTRIBUTE[MID]))
            {
                m_nAttribute = MID;
            }
            else if (m_strAttribute.Equals(ATTRIBUTE[MAX_SIZE]))
            {
                m_nAttribute = MAX_SIZE;
            }
            break;

        case 'o':
            if (m_strAttribute.Equals(ATTRIBUTE[ORIENT]))
            {
                m_nAttribute = ORIENT;
            }
            break;

        case 'p':
            if (m_strAttribute.Equals(ATTRIBUTE[PTIME]))
            {
                m_nAttribute = PTIME;
            }
            else if (m_strAttribute.Equals(ATTRIBUTE[PATH]))
            {
                m_nAttribute = PATH;
            }
            else if (m_strAttribute.Equals(ATTRIBUTE[PCFG]))
            {
                m_nAttribute = PCFG;
            }
            break;

        case 'q':
            if (m_strAttribute.Equals(ATTRIBUTE[QUALITY]))
            {
                m_nAttribute = QUALITY;
            }
            break;

        case 'r':
            if (m_strAttribute.Equals(ATTRIBUTE[RECVONLY]))
            {
                m_nAttribute = RECVONLY;
            }
            else if (m_strAttribute.Equals(ATTRIBUTE[RTCP]))
            {
                m_nAttribute = RTCP;
            }
            else if (m_strAttribute.Equals(ATTRIBUTE[RTPMAP]))
            {
                m_nAttribute = RTPMAP;
            }
            else if (m_strAttribute.Equals(ATTRIBUTE[RTCP_FB]))
            {
                m_nAttribute = RTCP_FB;
            }
            else if (m_strAttribute.Equals(ATTRIBUTE[RTCP_XR]))
            {
                m_nAttribute = RTCP_XR;
            }
            break;

        case 's':
            if (m_strAttribute.Equals(ATTRIBUTE[SDPLANG]))
            {
                m_nAttribute = SDPLANG;
            }
            else if (m_strAttribute.Equals(ATTRIBUTE[SENDONLY]))
            {
                m_nAttribute = SENDONLY;
            }
            else if (m_strAttribute.Equals(ATTRIBUTE[SENDRECV]))
            {
                m_nAttribute = SENDRECV;
            }
            else if (m_strAttribute.Equals(ATTRIBUTE[SETUP]))
            {
                m_nAttribute = SETUP;
            }
            break;

        case 't':
            if (m_strAttribute.Equals(ATTRIBUTE[TOOL]))
            {
                m_nAttribute = TOOL;
            }
            else if (m_strAttribute.Equals(ATTRIBUTE[TYPE]))
            {
                m_nAttribute = TYPE;
            }
            else if (m_strAttribute.Equals(ATTRIBUTE[TCAP]))
            {
                m_nAttribute = TCAP;
            }
            break;

        default:
            m_nAttribute = ATTRIBUTE_OTHER;
            break;
    }

    // value field
    if (m_nFormat == FORMAT_VALUE_ATTRIBUTE)
    {
        switch (IsValueToken(m_nAttribute, m_strAttribute))
        {
            case VALUE_TOKEN:
                if (!Sdp::IsTokenString(m_strAttrValue))
                {
                    // Invalid value field
                    return IMS_FALSE;
                }
                break;

            case VALUE_TOKEN_SP:
                if (!Sdp::IsTokenString(m_strAttrValue, IMS_TRUE))
                {
                    // Invalid value field
                    return IMS_FALSE;
                }
                break;

            default:
                // Check if the empty string is allowed
                if ((m_nAttribute == ACCEPT_TYPES) || (m_nAttribute == ACCEPT_WRAPPED_TYPES))
                {
                    if (m_strAttrValue.GetLength() == 0)
                    {
                        break;
                    }
                }

                if (!Sdp::IsTextString(m_strAttrValue))
                {
                    // Invalid value field
                    return IMS_FALSE;
                }
                break;
        }
    }

    return IMS_TRUE;
}

PUBLIC VIRTUAL AString SdpAttribute::Encode() const
{
    if (m_strAttribute.GetLength() == 0)
    {
        return AString::ConstNull();
    }

    // a=<attribute>
    // a=<attribute>:<value>
    AString strLine(1, Sdp::LINE_A);

    strLine.Append(TextParser::CHAR_EQUAL);
    strLine.Append(GetValue());
    strLine.Append(TextParser::CHAR_CR);
    strLine.Append(TextParser::CHAR_LF);

    return strLine;
}

PUBLIC VIRTUAL AString SdpAttribute::GetValue() const
{
    // a=<attribute>
    // a=<attribute>:<value>

    if (m_nFormat == FORMAT_PROPERTY_ATTRIBUTE)
    {
        return m_strAttribute;
    }

    AString strValue = m_strAttribute;

    strValue.Append(TextParser::CHAR_COLON);
    strValue.Append(m_strAttrValue);

    return strValue;
}

PUBLIC
IMS_BOOL SdpAttribute::Equals(IN const SdpAttribute* pAttribute) const
{
    if (pAttribute == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (m_nFormat != pAttribute->m_nFormat)
    {
        return IMS_FALSE;
    }

    if (m_nAttribute != pAttribute->m_nAttribute)
    {
        return IMS_FALSE;
    }

    if ((m_nAttribute == ATTRIBUTE_OTHER) && !m_strAttribute.Equals(pAttribute->m_strAttribute))
    {
        return IMS_FALSE;
    }

    // 4 case-sensitive ???
    if (!m_strAttrValue.Equals(pAttribute->m_strAttrValue))
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL SdpAttribute::SetValue(IN IMS_SINT32 nAttribute, IN const AString& strAttrValue,
        IN const AString& strAttribute /*= AString::ConstNull()*/)
{
    if ((nAttribute <= ATTRIBUTE_INVALID) || (nAttribute >= ATTRIBUTE_MAX))
    {
        return IMS_FALSE;
    }

    // attribute field
    if (nAttribute == ATTRIBUTE_OTHER)
    {
        if (!Sdp::IsTokenString(strAttribute))
        {
            // Invalid attribute field
            return IMS_FALSE;
        }
    }

    m_nAttribute = nAttribute;
    m_strAttribute = strAttribute;

    if (m_nAttribute != ATTRIBUTE_OTHER)
    {
        m_strAttribute = ATTRIBUTE[m_nAttribute];
    }

    // Set an attribute format
    if (strAttrValue.GetLength() == 0)
    {
        m_nFormat = FORMAT_PROPERTY_ATTRIBUTE;
        m_strAttrValue = AString::ConstEmpty();
    }
    else
    {
        m_nFormat = FORMAT_VALUE_ATTRIBUTE;
    }

    if (m_nFormat != GetFormat(m_nFormat, m_nAttribute, m_strAttribute))
    {
        // Format mismatched
        return IMS_FALSE;
    }

    // value field
    if (m_nFormat == FORMAT_VALUE_ATTRIBUTE)
    {
        switch (IsValueToken(m_nAttribute, m_strAttribute))
        {
            case VALUE_TOKEN:
                if (!Sdp::IsTokenString(strAttrValue))
                {
                    // Invalid value field
                    return IMS_FALSE;
                }
                break;

            case VALUE_TOKEN_SP:
                if (!Sdp::IsTokenString(strAttrValue, IMS_TRUE))
                {
                    // Invalid value field
                    return IMS_FALSE;
                }
                break;

            default:
                // Check if the empty string is allowed
                if ((nAttribute == ACCEPT_TYPES) || (nAttribute == ACCEPT_WRAPPED_TYPES))
                {
                    if (strAttrValue.GetLength() == 0)
                    {
                        break;
                    }
                }

                if (!Sdp::IsTextString(strAttrValue))
                {
                    // Invalid value field
                    return IMS_FALSE;
                }
                break;
        }

        m_strAttrValue = strAttrValue;
    }

    return IMS_TRUE;
}

PUBLIC GLOBAL IMS_SINT32 SdpAttribute::ConvertDirectionToAttribute(IN IMS_SINT32 nDirection)
{
    if (nDirection == Sdp::DIRECTION_INACTIVE)
    {
        return INACTIVE;
    }
    else if (nDirection == Sdp::DIRECTION_RECVONLY)
    {
        return RECVONLY;
    }
    else if (nDirection == Sdp::DIRECTION_SENDONLY)
    {
        return SENDONLY;
    }
    else if (nDirection == Sdp::DIRECTION_SENDRECV)
    {
        return SENDRECV;
    }
    else
    {
        return 0;  // DIRECTION_NONE
    }
}

PUBLIC GLOBAL IMS_SINT32 SdpAttribute::ConvertAttributeToDirection(IN IMS_SINT32 nAttribute)
{
    if (nAttribute == INACTIVE)
    {
        return Sdp::DIRECTION_INACTIVE;
    }
    else if (nAttribute == RECVONLY)
    {
        return Sdp::DIRECTION_RECVONLY;
    }
    else if (nAttribute == SENDONLY)
    {
        return Sdp::DIRECTION_SENDONLY;
    }
    else if (nAttribute == SENDRECV)
    {
        return Sdp::DIRECTION_SENDRECV;
    }
    else
    {
        return Sdp::DIRECTION_NONE;
    }
}

PUBLIC GLOBAL const IMS_CHAR* SdpAttribute::GetAttributeName(IN IMS_SINT32 nAttribute)
{
    if ((nAttribute > ATTRIBUTE_INVALID) && (nAttribute < ATTRIBUTE_ALL))
    {
        return ATTRIBUTE[nAttribute];
    }

    return IMS_NULL;
}

PRIVATE GLOBAL IMS_SINT32 SdpAttribute::IsValueToken(
        IN IMS_SINT32 nAttribute, IN const AString& strAttribute /*= AString::ConstNull()*/)
{
    (void)strAttribute;

    switch (nAttribute)
    {
        case CONTENT:
        case LABEL:
        case MID:
        case A_3GE2AE:
            return VALUE_TOKEN;

        case CONF:
        case CURR:
        case DES:
        case GROUP:
            return VALUE_TOKEN_SP;

        default:
            break;
    }

    return VALUE_NO_TOKEN;
}

PRIVATE GLOBAL IMS_SINT32 SdpAttribute::GetFormat(IN IMS_SINT32 nFormat, IN IMS_SINT32 nAttribute,
        IN const AString& strAttribute /*= AString::ConstNull()*/)
{
    (void)strAttribute;

    switch (nAttribute)
    {
        case INACTIVE:
        case RECVONLY:
        case SENDONLY:
        case SENDRECV:
            return FORMAT_PROPERTY_ATTRIBUTE;

        default:
            break;
    }

    return nFormat;
}
