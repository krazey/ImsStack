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
#include "ServiceTrace.h"
#include "TextParser.h"

#include "Sdp.h"
#include "SdpConnection.h"

__IMS_TRACE_TAG_SDP__;

PUBLIC
SdpConnection::SdpConnection() :
        SdpLine(),
        m_nNetType(Sdp::NET_TYPE_IN),
        m_strNetType(Sdp::STR_NET_TYPE_IN),
        m_nAddrType(Sdp::ADDR_TYPE_IP6),
        m_strAddrType(Sdp::STR_ADDR_TYPE_IP6),
        m_strAddress(AString::ConstNull()),
        m_nTtl(0),
        m_nNumOfAddress(0)
{
}

PUBLIC
SdpConnection::SdpConnection(IN const SdpConnection& other) :
        SdpLine(other),
        m_nNetType(other.m_nNetType),
        m_strNetType(other.m_strNetType),
        m_nAddrType(other.m_nAddrType),
        m_strAddrType(other.m_strAddrType),
        m_strAddress(other.m_strAddress),
        m_nTtl(other.m_nTtl),
        m_nNumOfAddress(other.m_nNumOfAddress)
{
}

PUBLIC VIRTUAL SdpConnection::~SdpConnection() {}

PUBLIC
SdpConnection& SdpConnection::operator=(IN const SdpConnection& other)
{
    if (this != &other)
    {
        SdpLine::operator=(other);

        m_nNetType = other.m_nNetType;
        m_strNetType = other.m_strNetType;
        m_nAddrType = other.m_nAddrType;
        m_strAddrType = other.m_strAddrType;
        m_strAddress = other.m_strAddress;
        m_nTtl = other.m_nTtl;
        m_nNumOfAddress = other.m_nNumOfAddress;
    }

    return (*this);
}

PUBLIC VIRTUAL IMS_BOOL SdpConnection::Decode(IN const AString& strValue)
{
    // c=<nettype> <addrtype> <connection-address>
    AStringArray objTokens;

    if (!Sdp::SplitLine(strValue, 3, objTokens))
    {
        // Invalid connection line
        return IMS_FALSE;
    }

    // nettype field
    m_strNetType = objTokens.GetElementAt(0);

    if (m_strNetType.Equals(Sdp::STR_NET_TYPE_IN))
    {
        m_nNetType = Sdp::NET_TYPE_IN;
    }
    else
    {
        if (!Sdp::IsTokenString(m_strNetType))
        {
            // Invalid nettype field
            return IMS_FALSE;
        }

        m_nNetType = Sdp::NET_TYPE_OTHER;
    }

    // addrtype field
    m_strAddrType = objTokens.GetElementAt(1);

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
            // Invalid addrtype field
            return IMS_FALSE;
        }

        m_nAddrType = Sdp::ADDR_TYPE_OTHER;
    }

    // TODO:: allow FQDN, so needs to be parsed using URI parser
    // TODO:: multicast ???
    // connection-address field
    ImsList<AString> objConnectionAddress = objTokens.GetElementAt(2).Split(TextParser::CHAR_SLASH);

    if (objConnectionAddress.IsEmpty())
    {
        return IMS_FALSE;
    }

    m_strAddress = objConnectionAddress.GetAt(0);

    if (objConnectionAddress.GetSize() > 1)
    {
        IMS_BOOL bOk = IMS_FALSE;

        m_nTtl = objConnectionAddress.GetAt(1).ToInt32(&bOk);

        if (!bOk)
        {
            // Number format invalid
            return IMS_FALSE;
        }

        if (objConnectionAddress.GetSize() == 3)
        {
            bOk = IMS_FALSE;

            m_nNumOfAddress = objConnectionAddress.GetAt(2).ToInt32(&bOk);

            if (!bOk)
            {
                // Number format invalid
                return IMS_FALSE;
            }
        }
    }

    // Check if the address format is valid
    if (!CheckValidityForAddress(m_strAddress, m_nAddrType))
    {
        IMS_TRACE_E(0, "c-line :: Address validity failed", 0, 0, 0);
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PUBLIC VIRTUAL AString SdpConnection::Encode() const
{
    if (!IsValid())
    {
        return AString::ConstNull();
    }

    // c=<nettype> <addrtype> <connection-address>
    AString strLine(1, Sdp::LINE_C);

    strLine.Append(TextParser::CHAR_EQUAL);
    strLine.Append(GetValue());
    strLine.Append(TextParser::CHAR_CR);
    strLine.Append(TextParser::CHAR_LF);

    return strLine;
}

PUBLIC VIRTUAL AString SdpConnection::GetValue() const
{
    if (!IsValid())
    {
        return AString::ConstNull();
    }

    AString strValue;

    // nettype
    strValue.Append(m_strNetType);
    strValue.Append(TextParser::CHAR_SP);

    // addrtype
    strValue.Append(m_strAddrType);
    strValue.Append(TextParser::CHAR_SP);

    // connection-address
    strValue.Append(m_strAddress);

    // TODO:: multicast address

    return strValue;
}

PUBLIC
ImsList<AString> SdpConnection::GetAddresses() const
{
    ImsList<AString> objAddresses;
    objAddresses.Append(m_strAddress);
    // TODO: add extra addresses
    return objAddresses;
}

PUBLIC
IMS_BOOL SdpConnection::SetValue(IN IMS_SINT32 nAddrType, IN const AString& strAddress,
        IN const AString& strOtherAddrType /* = AString::ConstNull() */,
        IN IMS_SINT32 nTtl /* = 0 */, IN IMS_SINT32 nNumOfAddress /* = 0 */)
{
    // nettype
    m_nNetType = Sdp::NET_TYPE_IN;
    m_strNetType = Sdp::STR_NET_TYPE_IN;

    // addrtype
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
                // Invalid addrtype field
                return IMS_FALSE;
            }

            m_strAddrType = strOtherAddrType;
            break;

        default:
            return IMS_FALSE;
    }

    m_nTtl = nTtl;
    m_nNumOfAddress = nNumOfAddress;

    // Check if the address format is valid
    if (!CheckValidityForAddress(strAddress, m_nAddrType))
    {
        IMS_TRACE_E(0, "c-line :: Address validity failed", 0, 0, 0);
        return IMS_FALSE;
    }

    m_strAddress = strAddress;

    return IMS_TRUE;
}

PRIVATE IMS_BOOL SdpConnection::IsValid() const
{
    return m_strNetType.GetLength() != 0 && m_strAddrType.GetLength() != 0 &&
            m_strAddress.GetLength() != 0;
}
