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
#include "AStringArray.h"
#include "IpAddress.h"
#include "ServiceSystemTime.h"
#include "ServiceTrace.h"
#include "TextParser.h"

#include "Sdp.h"
#include "SdpOrigin.h"

__IMS_TRACE_TAG_SDP__;

PUBLIC GLOBAL const IMS_CHAR SdpOrigin::DEFAULT_USERNAME[] = "-";
PRIVATE GLOBAL IMS_UINT32 SdpOrigin::s_nLastTime = 0;

PUBLIC
SdpOrigin::SdpOrigin() :
        SdpLine(),
        m_strUsername(AString::ConstNull()),
        m_strSessionId(AString::ConstNull()),
        m_strSessionVersion(AString::ConstNull()),
        m_nNetType(Sdp::NET_TYPE_IN),
        m_strNetType(Sdp::STR_NET_TYPE_IN),
        m_nAddrType(Sdp::ADDR_TYPE_IP6),
        m_strAddrType(Sdp::STR_ADDR_TYPE_IP6),
        m_strUnicastAddress(AString::ConstNull())
{
}

PUBLIC
SdpOrigin::SdpOrigin(IN const SdpOrigin& other) :
        SdpLine(other),
        m_strUsername(other.m_strUsername),
        m_strSessionId(other.m_strSessionId),
        m_strSessionVersion(other.m_strSessionVersion),
        m_nNetType(other.m_nNetType),
        m_strNetType(other.m_strNetType),
        m_nAddrType(other.m_nAddrType),
        m_strAddrType(other.m_strAddrType),
        m_strUnicastAddress(other.m_strUnicastAddress)
{
}

PUBLIC VIRTUAL SdpOrigin::~SdpOrigin() {}

PUBLIC
SdpOrigin& SdpOrigin::operator=(IN const SdpOrigin& other)
{
    if (this != &other)
    {
        SdpLine::operator=(other);

        m_strUsername = other.m_strUsername;
        m_strSessionId = other.m_strSessionId;
        m_strSessionVersion = other.m_strSessionVersion;
        m_nNetType = other.m_nNetType;
        m_strNetType = other.m_strNetType;
        m_nAddrType = other.m_nAddrType;
        m_strAddrType = other.m_strAddrType;
        m_strUnicastAddress = other.m_strUnicastAddress;
    }

    return (*this);
}

PUBLIC VIRTUAL IMS_BOOL SdpOrigin::Decode(IN const AString& strValue)
{
    // o=<username> <sess-id> <sess-version> <nettype> <addrtype> <unicast-address>
    AStringArray objTokens;

    if (!Sdp::SplitLine(strValue, 6, objTokens))
    {
        IMS_TRACE_E(0, "Invalid origin line: %s", strValue.GetStr(), 0, 0);
        return IMS_FALSE;
    }

    // username field
    if (!Sdp::IsNonWsString(objTokens.GetElementAt(0)))
    {
        IMS_TRACE_E(0, "Invalid username field of origin line: %s", strValue.GetStr(), 0, 0);
        return IMS_FALSE;
    }

    m_strUsername = objTokens.GetElementAt(0);

    // sess-id field
    if (!Sdp::IsDigitString(objTokens.GetElementAt(1)))
    {
        IMS_TRACE_E(0, "Invalid sess-id field of origin line: %s", strValue.GetStr(), 0, 0);
        return IMS_FALSE;
    }

    m_strSessionId = objTokens.GetElementAt(1);

    // sess-version field
    if (!Sdp::IsDigitString(objTokens.GetElementAt(2)))
    {
        IMS_TRACE_E(0, "Invalid sess-version field of origin line: %s", strValue.GetStr(), 0, 0);
        return IMS_FALSE;
    }

    m_strSessionVersion = objTokens.GetElementAt(2);

    // nettype field
    m_strNetType = objTokens.GetElementAt(3);

    if (m_strNetType.Equals(Sdp::STR_NET_TYPE_IN))
    {
        m_nNetType = Sdp::NET_TYPE_IN;
    }
    else
    {
        if (!Sdp::IsTokenString(m_strNetType))
        {
            IMS_TRACE_E(0, "Invalid nettype field of origin line: %s", strValue.GetStr(), 0, 0);
            return IMS_FALSE;
        }

        m_nNetType = Sdp::NET_TYPE_OTHER;
    }

    // addrtype field
    m_strAddrType = objTokens.GetElementAt(4);

    if (m_strAddrType.Equals(Sdp::STR_ADDR_TYPE_IP4))
    {
        m_nAddrType = Sdp::ADDR_TYPE_IP4;
    }
    else if (m_strAddrType.Equals(Sdp::STR_ADDR_TYPE_IP6))
    {
        m_nAddrType = Sdp::ADDR_TYPE_IP6;
    }
    else
    {
        if (!Sdp::IsTokenString(m_strAddrType))
        {
            IMS_TRACE_E(0, "Invalid addrtype field of origin line: %s", strValue.GetStr(), 0, 0);
            return IMS_FALSE;
        }

        m_nAddrType = Sdp::ADDR_TYPE_OTHER;
    }

    // To allow FQDN, so needs to be parsed using URI parser
    // unicast-address field
    m_strUnicastAddress = objTokens.GetElementAt(5);

    // Check if the address format is valid
    if (!CheckValidityForAddress(m_strUnicastAddress, m_nAddrType))
    {
        IMS_TRACE_E(0, "o-line :: Address validity failed : %s", strValue.GetStr(), 0, 0);
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PUBLIC VIRTUAL AString SdpOrigin::Encode() const
{
    if (!IsValid())
    {
        return AString::ConstNull();
    }

    // o=<username> <sess-id> <sess-version> <nettype> <addrtype> <unicast-address>
    AString strLine(1, Sdp::LINE_O);

    strLine.Append(TextParser::CHAR_EQUAL);
    strLine.Append(GetValue());
    strLine.Append(TextParser::CHAR_CR);
    strLine.Append(TextParser::CHAR_LF);

    return strLine;
}

PUBLIC VIRTUAL AString SdpOrigin::GetValue() const
{
    if (!IsValid())
    {
        return AString::ConstNull();
    }

    AString strValue;

    // username field
    strValue.Append(m_strUsername);
    strValue.Append(TextParser::CHAR_SP);

    // sess-id field
    strValue.Append(m_strSessionId);
    strValue.Append(TextParser::CHAR_SP);

    // sess-version field
    strValue.Append(m_strSessionVersion);
    strValue.Append(TextParser::CHAR_SP);

    // nettype field
    strValue.Append(m_strNetType);
    strValue.Append(TextParser::CHAR_SP);

    // addrtype field
    strValue.Append(m_strAddrType);
    strValue.Append(TextParser::CHAR_SP);

    // unicast-address
    strValue.Append(m_strUnicastAddress);

    return strValue;
}

PUBLIC
void SdpOrigin::IncreaseSessionVersion()
{
    m_strSessionVersion = Sdp::IncreaseSessionVersion(m_strSessionVersion);
}

PUBLIC
IMS_BOOL SdpOrigin::SetAddress(IN const AString& strAddress)
{
    IpAddress objAddress;

    if (!objAddress.Parse(strAddress))
    {
        if (!Sdp::IsFqdnString(strAddress) && !Sdp::IsNonWsString(strAddress))
        {
            IMS_TRACE_E(0, "Invalid connection-address: %s", strAddress.GetStr(), 0, 0);
            return IMS_FALSE;
        }

        m_nAddrType = Sdp::ADDR_TYPE_IP6;
        m_strAddrType = Sdp::STR_ADDR_TYPE_IP6;
    }
    else
    {
        // addrtype field
        if (objAddress.IsIPv4Address())
        {
            m_nAddrType = Sdp::ADDR_TYPE_IP4;
            m_strAddrType = Sdp::STR_ADDR_TYPE_IP4;
        }
        else
        {
            m_nAddrType = Sdp::ADDR_TYPE_IP6;
            m_strAddrType = Sdp::STR_ADDR_TYPE_IP6;
        }
    }

    // unicast-address field
    m_strUnicastAddress = strAddress;

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL SdpOrigin::SetValue(IN const AString& strUsername, IN const AString& strAddress)
{
    if (!Sdp::IsNonWsString(strUsername))
    {
        IMS_TRACE_E(0, "Invalid username: %s", strUsername.GetStr(), 0, 0);
        return IMS_FALSE;
    }

    IpAddress objAddress;

    if (!objAddress.Parse(strAddress))
    {
        if (!Sdp::IsFqdnString(strAddress) && !Sdp::IsNonWsString(strAddress))
        {
            IMS_TRACE_E(0, "Invalid connection-address: %s", strAddress.GetStr(), 0, 0);
            return IMS_FALSE;
        }

        m_nAddrType = Sdp::ADDR_TYPE_IP6;
        m_strAddrType = Sdp::STR_ADDR_TYPE_IP6;
    }
    else
    {
        // addrtype field
        if (objAddress.IsIPv4Address())
        {
            m_nAddrType = Sdp::ADDR_TYPE_IP4;
            m_strAddrType = Sdp::STR_ADDR_TYPE_IP4;
        }
        else
        {
            m_nAddrType = Sdp::ADDR_TYPE_IP6;
            m_strAddrType = Sdp::STR_ADDR_TYPE_IP6;
        }
    }

    // username field
    m_strUsername = strUsername;

    // sess-id field
    m_strSessionId.SetNumber(GetNtpTime());

    // sess-version field
    m_strSessionVersion = m_strSessionId;

    // nettype field
    m_nNetType = Sdp::NET_TYPE_IN;
    m_strNetType = Sdp::STR_NET_TYPE_IN;

    // unicast-address field
    m_strUnicastAddress = strAddress;

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL SdpOrigin::SetValue(IN const AString& strUsername, IN const AString& strSessionId,
        IN const AString& strSessionVersion, IN IMS_SINT32 nAddrType, IN const AString& strAddress,
        IN const AString& strOtherAddrType /*= AString::ConstNull()*/)
{
    if (!Sdp::IsNonWsString(strUsername))
    {
        IMS_TRACE_E(0, "Invalid username: %s", strUsername.GetStr(), 0, 0);
        return IMS_FALSE;
    }

    if (!Sdp::IsDigitString(strSessionId))
    {
        IMS_TRACE_E(0, "Invalid sess-id: %s", strSessionId.GetStr(), 0, 0);
        return IMS_FALSE;
    }

    if (!Sdp::IsDigitString(strSessionVersion))
    {
        IMS_TRACE_E(0, "Invalid sess-version: %s", strSessionVersion.GetStr(), 0, 0);
        return IMS_FALSE;
    }

    // username field
    m_strUsername = strUsername;

    // sess-id field
    m_strSessionId = strSessionId;

    // sess-version field
    m_strSessionVersion = strSessionVersion;

    // nettype field
    m_nNetType = Sdp::NET_TYPE_IN;
    m_strNetType = Sdp::STR_NET_TYPE_IN;

    // addrtype field
    m_nAddrType = nAddrType;

    switch (m_nAddrType)
    {
        case Sdp::ADDR_TYPE_IP4:
            m_strAddrType = Sdp::STR_ADDR_TYPE_IP4;
            break;

        case Sdp::ADDR_TYPE_IP6:
            m_strAddrType = Sdp::STR_ADDR_TYPE_IP6;
            break;

        case Sdp::ADDR_TYPE_OTHER:
            if (!Sdp::IsTokenString(strOtherAddrType))
            {
                IMS_TRACE_E(0, "Invalid addrtype: %s", strOtherAddrType.GetStr(), 0, 0);
                return IMS_FALSE;
            }

            m_strAddrType = strOtherAddrType;
            break;

        default:
            return IMS_FALSE;
    }

    // Check if the address format is valid
    if (!CheckValidityForAddress(strAddress, m_nAddrType))
    {
        IMS_TRACE_E(0, "o-line :: Address validity failed : %s", strAddress.GetStr(), 0, 0);
        return IMS_FALSE;
    }

    // unicast-address field
    m_strUnicastAddress = strAddress;

    return IMS_TRUE;
}

PRIVATE IMS_BOOL SdpOrigin::IsValid() const
{
    return m_strUsername.GetLength() != 0 && m_strSessionId.GetLength() != 0 &&
            m_strSessionVersion.GetLength() != 0 && m_strNetType.GetLength() != 0 &&
            m_strAddrType.GetLength() != 0 && m_strUnicastAddress.GetLength() != 0;
}

PRIVATE GLOBAL IMS_UINT32 SdpOrigin::GetNtpTime()
{
    IMS_UINT32 nCurrTime = IMS_SYS_GetTimeInSeconds();

    if ((nCurrTime == s_nLastTime) && (s_nLastTime != 0))
    {
        return nCurrTime;
    }

    s_nLastTime = nCurrTime;

    return (nCurrTime + NTP_OFFSET);
}
