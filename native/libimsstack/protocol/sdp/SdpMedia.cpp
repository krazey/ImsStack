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
#include "ServiceMemory.h"

#include "Sdp.h"
#include "SdpMedia.h"

PUBLIC GLOBAL const IMS_CHAR* SdpMedia::MEDIA[SdpMedia::TYPE_MAX] = {
        "audio",
        "video",
        "text",
        "application",
        "message",
        IMS_NULL,
};

PUBLIC GLOBAL const IMS_CHAR* SdpMedia::TRANSPORT[SdpMedia::TRANSPORT_MAX] = {
        "udp",
        "RTP/AVP",
        "RTP/AVPF",
        "RTP/SAVP",
        "RTP/SAVPF",
        "UDP/TLS/RTP/SAVP",
        "tcp",
        "TCP/MSRP",
        "TCP/TLS/MSRP",
        IMS_NULL,
};

PUBLIC
SdpMedia::SdpMedia() :
        SdpLine(),
        m_nMediaType(TYPE_INVALID),
        m_strMediaType(AString::ConstNull()),
        m_nPort(0),
        m_nNumOfPort(0),
        m_nTransportProtocol(TRANSPORT_INVALID),
        m_strTransportProtocol(AString::ConstNull())
{
}

PUBLIC
SdpMedia::SdpMedia(IN const SdpMedia& other) :
        SdpLine(other),
        m_nMediaType(other.m_nMediaType),
        m_strMediaType(other.m_strMediaType),
        m_nPort(other.m_nPort),
        m_nNumOfPort(other.m_nNumOfPort),
        m_nTransportProtocol(other.m_nTransportProtocol),
        m_strTransportProtocol(other.m_strTransportProtocol),
        m_objFormats(other.m_objFormats)
{
}

PUBLIC VIRTUAL SdpMedia::~SdpMedia() {}

PUBLIC
SdpMedia& SdpMedia::operator=(IN const SdpMedia& other)
{
    if (this != &other)
    {
        SdpLine::operator=(other);

        m_nMediaType = other.m_nMediaType;
        m_strMediaType = other.m_strMediaType;
        m_nPort = other.m_nPort;
        m_nNumOfPort = other.m_nNumOfPort;
        m_nTransportProtocol = other.m_nTransportProtocol;
        m_strTransportProtocol = other.m_strTransportProtocol;
        m_objFormats = other.m_objFormats;
    }

    return (*this);
}

PUBLIC VIRTUAL IMS_BOOL SdpMedia::Decode(IN const AString& strValue)
{
    // m=<media> <port> <protocol> <fmt> ...
    AStringArray objTokens;

    if (!Sdp::SplitLine(strValue, 4, objTokens))
    {
        // Invalid media field
        return IMS_FALSE;
    }

    // Media type info.
    m_strMediaType = objTokens.GetElementAt(0);
    m_nMediaType = TYPE_OTHER;

    IMS_SINT32 nType;

    for (nType = TYPE_INVALID + 1; nType < TYPE_OTHER; ++nType)
    {
        if (m_strMediaType.EqualsIgnoreCase(MEDIA[nType]))
        {
            m_nMediaType = nType;
            break;
        }
    }

    if (m_nMediaType == TYPE_OTHER)
    {
        if (!Sdp::IsTokenString(m_strMediaType))
        {
            // Invalid SDP value
            return IMS_FALSE;
        }
    }

    // Port info.
    const AString& strPortInfo = objTokens.GetElementAt(1);
    IMS_SINT32 nSlashIndex = strPortInfo.GetIndexOf(TextParser::CHAR_SLASH);

    if (nSlashIndex == AString::NPOS)
    {
        IMS_BOOL bOk = IMS_FALSE;

        m_nPort = strPortInfo.ToInt32(&bOk);

        if (!bOk)
        {
            // Invalid Port info.
            return IMS_FALSE;
        }
    }
    else
    {
        AString strPort = strPortInfo.GetSubStr(0, nSlashIndex);
        AString strNumOfPort = strPortInfo.GetSubStr(nSlashIndex + 1);
        IMS_BOOL bOk = IMS_FALSE;

        m_nPort = strPort.ToInt32(&bOk);

        if (!bOk)
        {
            // Invalid Port info.
            return IMS_FALSE;
        }

        bOk = IMS_FALSE;
        m_nNumOfPort = strNumOfPort.ToInt32(&bOk);

        if (!bOk)
        {
            // Invalid Port info.
            return IMS_FALSE;
        }
    }

    // Transport protocol info.
    m_strTransportProtocol = objTokens.GetElementAt(2);
    m_nTransportProtocol = TRANSPORT_OTHER;

    for (nType = TRANSPORT_INVALID + 1; nType < TRANSPORT_OTHER; ++nType)
    {
        if (m_strTransportProtocol.EqualsIgnoreCase(TRANSPORT[nType]))
        {
            m_nTransportProtocol = nType;
            break;
        }
    }

    // proto := token *("/" token)
    if (m_nTransportProtocol == TRANSPORT_OTHER)
    {
        if (m_strTransportProtocol.Contains(TextParser::CHAR_SLASH))
        {
            IMSList<AString> objProtoTokens = m_strTransportProtocol.Split(TextParser::CHAR_SLASH);

            for (IMS_UINT32 i = 0; i < objProtoTokens.GetSize(); ++i)
            {
                const AString& strToken = objProtoTokens.GetAt(i);

                if (!Sdp::IsTokenString(strToken))
                {
                    // Invalid SDP value
                    return IMS_FALSE;
                }
            }
        }
        else
        {
            if (!Sdp::IsTokenString(m_strTransportProtocol))
            {
                // Invalid SDP value
                return IMS_FALSE;
            }
        }
    }

    IMSList<AString> objPayloadTokens = objTokens.GetElementAt(3).Split(TextParser::CHAR_SP);

    for (IMS_UINT32 i = 0; i < objPayloadTokens.GetSize(); ++i)
    {
        const AString& strFormat = objPayloadTokens.GetAt(i);

        // Format (Payload Types) info.
        if (!Sdp::IsTokenString(strFormat))
        {
            // Invalid SDP value
            return IMS_FALSE;
        }

        m_objFormats.AddElement(strFormat);
    }

    return IMS_TRUE;
}

PUBLIC VIRTUAL AString SdpMedia::Encode() const
{
    // m=<media> <port> <protocol> <fmt> ...
    AString strLine(1, Sdp::LINE_M);

    strLine.Append(TextParser::CHAR_EQUAL);
    strLine.Append(GetValue());
    strLine.Append(TextParser::CHAR_CR);
    strLine.Append(TextParser::CHAR_LF);

    return strLine;
}

PUBLIC VIRTUAL AString SdpMedia::GetValue() const
{
    // m=<media> <port> <protocol> <fmt> ...
    AString strValue;

    // Media type
    strValue.Append(m_strMediaType);

    // Port[/Number of Ports]
    AString strPort;

    strPort.SetNumber(m_nPort);

    if (m_nNumOfPort != 0)
    {
        AString strNumOfPorts;

        strNumOfPorts.SetNumber(m_nNumOfPort);

        strPort.Append(TextParser::CHAR_SLASH);
        strPort.Append(strNumOfPorts);
    }

    strValue.Append(TextParser::CHAR_SP);
    strValue.Append(strPort);

    // Transport protocol
    strValue.Append(TextParser::CHAR_SP);
    strValue.Append(m_strTransportProtocol);

    // Payload types
    for (IMS_SINT32 i = 0; i < m_objFormats.GetCount(); ++i)
    {
        strValue.Append(TextParser::CHAR_SP);
        strValue.Append(m_objFormats.GetElementAt(i));
    }

    return strValue;
}

PUBLIC
IMS_BOOL SdpMedia::SetType(
        IN IMS_SINT32 nType, IN const AString& strOtherType /*= AString::ConstNull()*/)
{
    if ((nType <= TYPE_INVALID) || (nType >= TYPE_MAX))
    {
        return IMS_FALSE;
    }

    // Media type
    m_nMediaType = nType;

    if (m_nMediaType == TYPE_OTHER)
    {
        if (!Sdp::IsTokenString(strOtherType))
        {
            return IMS_FALSE;
        }

        m_strMediaType = strOtherType;
    }
    else
    {
        m_strMediaType = MEDIA[m_nMediaType];
    }

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL SdpMedia::SetTransportProtocol(IN IMS_SINT32 nTransportProtocol,
        IN const AString& strOtherTransportProtocol /*= AString::ConstNull()*/)
{
    if ((nTransportProtocol <= TRANSPORT_INVALID) || (nTransportProtocol >= TRANSPORT_MAX))
    {
        return IMS_FALSE;
    }

    m_nTransportProtocol = nTransportProtocol;

    if (m_nTransportProtocol == TRANSPORT_OTHER)
    {
        if (!Sdp::IsTokenString(strOtherTransportProtocol))
        {
            return IMS_FALSE;
        }

        m_strTransportProtocol = strOtherTransportProtocol;
    }
    else
    {
        m_strTransportProtocol = TRANSPORT[m_nTransportProtocol];
    }

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL SdpMedia::SetFormats(IN const AStringArray& objFormats)
{
    // Format (Payload Types) info.
    for (IMS_SINT32 i = 0; i < objFormats.GetCount(); ++i)
    {
        if (!Sdp::IsTokenString(objFormats.GetElementAt(i)))
        {
            return IMS_FALSE;
        }
    }

    m_objFormats = objFormats;

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL SdpMedia::SetValue(IN IMS_SINT32 nType, IN IMS_SINT32 nPort,
        IN IMS_SINT32 nTransportProtocol, IN const AStringArray& objFormats,
        IN const AString& strOtherType /*= AString::ConstNull()*/,
        IN const AString& strOtherTransportProtocol /*= AString::ConstNull()*/,
        IN IMS_SINT32 nNumOfPort /*= 0*/)
{
    if ((nType <= TYPE_INVALID) || (nType >= TYPE_MAX))
    {
        return IMS_FALSE;
    }

    if ((nTransportProtocol <= TRANSPORT_INVALID) || (nTransportProtocol >= TRANSPORT_MAX))
    {
        return IMS_FALSE;
    }

    // Media type
    m_nMediaType = nType;

    if (m_nMediaType == TYPE_OTHER)
    {
        if (!Sdp::IsTokenString(strOtherType))
        {
            return IMS_FALSE;
        }

        m_strMediaType = strOtherType;
    }
    else
    {
        m_strMediaType = MEDIA[m_nMediaType];
    }

    // Port info.
    m_nPort = nPort;
    m_nNumOfPort = nNumOfPort;

    // Transport protocol info.
    m_nTransportProtocol = nTransportProtocol;

    if (m_nTransportProtocol == TRANSPORT_OTHER)
    {
        if (!Sdp::IsTokenString(strOtherTransportProtocol))
        {
            return IMS_FALSE;
        }

        m_strTransportProtocol = strOtherTransportProtocol;
    }
    else
    {
        m_strTransportProtocol = TRANSPORT[m_nTransportProtocol];
    }

    // Format (Payload Types) info.
    for (IMS_SINT32 i = 0; i < objFormats.GetCount(); ++i)
    {
        if (!Sdp::IsTokenString(objFormats.GetElementAt(i)))
        {
            return IMS_FALSE;
        }
    }

    m_objFormats = objFormats;

    return IMS_TRUE;
}
