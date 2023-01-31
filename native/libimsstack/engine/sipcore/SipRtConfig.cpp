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

#include "SipRtConfig.h"

// LogMask class for SIP run-time (or real-time) configuration
PUBLIC
SipRtConfig::LogMask::LogMask() :
        Base(),
        nValue(LogMask::I_NONE)
{
}

PUBLIC
SipRtConfig::LogMask::~LogMask() {}

// SocketOption class for SIP run-time (or real-time) configuration
PUBLIC
SipRtConfig::SocketOption::SocketOption() :
        Base(),
        nValue(0),
        objIpAddr(IpAddress::NONE),
        nPort(0)
{
}

PUBLIC
SipRtConfig::SocketOption::SocketOption(IN const SipRtConfig::SocketOption& other) :
        Base(other),
        nValue(other.nValue),
        objIpAddr(other.objIpAddr),
        nPort(other.nPort)
{
}

PUBLIC VIRTUAL SipRtConfig::SocketOption::~SocketOption() {}

PUBLIC
SipRtConfig::SocketOption& SipRtConfig::SocketOption::operator=(
        IN const SipRtConfig::SocketOption& other)
{
    if (this != &other)
    {
        nValue = other.nValue;
        objIpAddr = other.objIpAddr;
        nPort = other.nPort;
    }

    return (*this);
}

PUBLIC VIRTUAL IMS_BOOL SipRtConfig::SocketOption::Equals(IN const SipRtConfig::Base& other) const
{
    const SipRtConfig::SocketOption* pSo = DYNAMIC_CAST(const SipRtConfig::SocketOption*, &other);

    return (pSo != IMS_NULL) ? (objIpAddr.Equals(pSo->objIpAddr) && (nPort == pSo->nPort))
                             : IMS_FALSE;
}

PUBLIC
IMS_BOOL SipRtConfig::SocketOption::IsGlobalOption() const
{
    return ((objIpAddr.Equals(IpAddress::NONE) || objIpAddr.Equals(IpAddress::IPv6NONE)) &&
            (nPort == 0));
}

// IpQos class for IP-level QoS configuration
PUBLIC
SipRtConfig::IpQos::IpQos() :
        Base(),
        nValue(0),
        objIpAddr(IpAddress::NONE),
        nPort(0)
{
}

PUBLIC
SipRtConfig::IpQos::IpQos(IN const SipRtConfig::IpQos& other) :
        Base(other),
        nValue(other.nValue),
        objIpAddr(other.objIpAddr),
        nPort(other.nPort)
{
}

PUBLIC VIRTUAL SipRtConfig::IpQos::~IpQos() {}

PUBLIC
SipRtConfig::IpQos& SipRtConfig::IpQos::operator=(IN const SipRtConfig::IpQos& other)
{
    if (this != &other)
    {
        nValue = other.nValue;
        objIpAddr = other.objIpAddr;
        nPort = other.nPort;
    }

    return (*this);
}

PUBLIC VIRTUAL IMS_BOOL SipRtConfig::IpQos::Equals(IN const SipRtConfig::Base& other) const
{
    const SipRtConfig::IpQos* pIpQos = DYNAMIC_CAST(const SipRtConfig::IpQos*, &other);

    return (pIpQos != IMS_NULL) ? (objIpAddr.Equals(pIpQos->objIpAddr) && (nPort == pIpQos->nPort))
                                : IMS_FALSE;
}

// Header class for an additional SIP header control
PUBLIC
SipRtConfig::Header::Header() :
        Base(),
        strName(AString::ConstNull()),
        strParameter(AString::ConstNull())
{
}

PUBLIC
SipRtConfig::Header::Header(IN const SipRtConfig::Header& other) :
        Base(other),
        strName(other.strName),
        strParameter(other.strParameter)
{
}

PUBLIC VIRTUAL SipRtConfig::Header::~Header() {}

PUBLIC
SipRtConfig::Header& SipRtConfig::Header::operator=(IN const SipRtConfig::Header& other)
{
    if (this != &other)
    {
        strName = other.strName;
        strParameter = other.strParameter;
    }

    return (*this);
}

PUBLIC VIRTUAL IMS_BOOL SipRtConfig::Header::Equals(IN const SipRtConfig::Base& other) const
{
    const SipRtConfig::Header* pHeader = DYNAMIC_CAST(const SipRtConfig::Header*, &other);

    if (pHeader == IMS_NULL)
    {
        return IMS_FALSE;
    }

    return strName.EqualsIgnoreCase(pHeader->strName);
}

// IpSecSa class to do the SA's validity check
PUBLIC
SipRtConfig::IpSecSa::IpSecSa() :
        Base(),
        nPortPc(0),
        nPortPs(0),
        objIpAddrP(IpAddress::NONE),
        nPortUc(0),
        nPortUs(0),
        objIpAddrU(IpAddress::NONE)
{
}

PUBLIC
SipRtConfig::IpSecSa::IpSecSa(IN const SipRtConfig::IpSecSa& other) :
        Base(other),
        nPortPc(other.nPortPc),
        nPortPs(other.nPortPs),
        objIpAddrP(other.objIpAddrP),
        nPortUc(other.nPortUc),
        nPortUs(other.nPortUs),
        objIpAddrU(other.objIpAddrU)
{
}

PUBLIC VIRTUAL SipRtConfig::IpSecSa::~IpSecSa() {}

PUBLIC
SipRtConfig::IpSecSa& SipRtConfig::IpSecSa::operator=(IN const SipRtConfig::IpSecSa& other)
{
    if (this != &other)
    {
        nPortPc = other.nPortPc;
        nPortPs = other.nPortPs;
        objIpAddrP = other.objIpAddrP;
        nPortUc = other.nPortUc;
        nPortUs = other.nPortUs;
        objIpAddrU = other.objIpAddrU;
    }

    return (*this);
}

PUBLIC VIRTUAL IMS_BOOL SipRtConfig::IpSecSa::Equals(IN const SipRtConfig::Base& other) const
{
    const SipRtConfig::IpSecSa* pIpSecSa = DYNAMIC_CAST(const SipRtConfig::IpSecSa*, &other);

    if (pIpSecSa == IMS_NULL)
    {
        return IMS_FALSE;
    }

    return (nPortPc == pIpSecSa->nPortPc) && (nPortPs == pIpSecSa->nPortPs) &&
            objIpAddrP.Equals(pIpSecSa->objIpAddrP) && (nPortUc == pIpSecSa->nPortUc) &&
            (nPortUs == pIpSecSa->nPortUs) && objIpAddrU.Equals(pIpSecSa->objIpAddrU);
}

PUBLIC
IMS_BOOL SipRtConfig::IpSecSa::IsEmpty() const
{
    return (nPortPc == 0) && (nPortPs == 0) && (nPortUc == 0) && (nPortUs == 0);
}

// TcpPortRange class to provision TCP port range
PUBLIC
SipRtConfig::TcpPortRange::TcpPortRange() :
        Base(),
        nPortStart(0),
        nPortEnd(0)
{
}

PUBLIC
SipRtConfig::TcpPortRange::TcpPortRange(IN const SipRtConfig::TcpPortRange& other) :
        Base(other),
        nPortStart(other.nPortStart),
        nPortEnd(other.nPortEnd)
{
}

PUBLIC VIRTUAL SipRtConfig::TcpPortRange::~TcpPortRange() {}

PUBLIC
SipRtConfig::TcpPortRange& SipRtConfig::TcpPortRange::operator=(
        IN const SipRtConfig::TcpPortRange& other)
{
    if (this != &other)
    {
        nPortStart = other.nPortStart;
        nPortEnd = other.nPortEnd;
    }

    return (*this);
}

PUBLIC VIRTUAL IMS_BOOL SipRtConfig::TcpPortRange::Equals(IN const SipRtConfig::Base& other) const
{
    const SipRtConfig::TcpPortRange* pPortRange =
            DYNAMIC_CAST(const SipRtConfig::TcpPortRange*, &other);

    if (pPortRange == IMS_NULL)
    {
        return IMS_FALSE;
    }

    return (nPortStart == pPortRange->nPortStart) && (nPortEnd == pPortRange->nPortEnd);
}

// RegContactAddress class to provision RegContact's information
PUBLIC
SipRtConfig::RegContactAddress::RegContactAddress() :
        Base(),
        strCallId(AString::ConstNull()),
        objUri(SipAddress::ConstNull())
{
}

PUBLIC
SipRtConfig::RegContactAddress::RegContactAddress(IN const SipRtConfig::RegContactAddress& other) :
        Base(other),
        strCallId(other.strCallId),
        objUri(other.objUri)
{
}

PUBLIC VIRTUAL SipRtConfig::RegContactAddress::~RegContactAddress() {}

PUBLIC
SipRtConfig::RegContactAddress& SipRtConfig::RegContactAddress::operator=(
        IN const SipRtConfig::RegContactAddress& other)
{
    if (this != &other)
    {
        strCallId = other.strCallId;
        objUri = other.objUri;
    }

    return (*this);
}

PUBLIC VIRTUAL IMS_BOOL SipRtConfig::RegContactAddress::Equals(
        IN const SipRtConfig::Base& other) const
{
    const SipRtConfig::RegContactAddress* pRegContactA =
            DYNAMIC_CAST(const SipRtConfig::RegContactAddress*, &other);

    if (pRegContactA == IMS_NULL)
    {
        return IMS_FALSE;
    }

    return strCallId.Equals(pRegContactA->strCallId);
}
