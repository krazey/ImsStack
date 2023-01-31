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

#include "SipPrivate.h"
#include "SipTransportAddress.h"

PUBLIC
SipTransportAddress::SipTransportAddress() :
        m_nProtocol(PROTOCOL_UDP),
        m_nPort(Sip::PORT_UNSPECIFIED),
        m_objIpAddr(IpAddress::NONE)
{
}

PUBLIC
SipTransportAddress::SipTransportAddress(IN const SipTransportAddress& other) :
        m_nProtocol(other.m_nProtocol),
        m_nPort(other.m_nPort),
        m_objIpAddr(other.m_objIpAddr)
{
}

PUBLIC
SipTransportAddress::SipTransportAddress(
        IN IMS_SINT32 nProtocol, IN IMS_SINT32 nPort, IN const AString& strAddress) :
        m_nProtocol(nProtocol),
        m_nPort(nPort)
{
    m_objIpAddr.Parse(strAddress);
}

PUBLIC
SipTransportAddress::~SipTransportAddress() {}

PUBLIC
SipTransportAddress& SipTransportAddress::operator=(IN const SipTransportAddress& other)
{
    if (this != &other)
    {
        m_nProtocol = other.m_nProtocol;
        m_nPort = other.m_nPort;
        m_objIpAddr = other.m_objIpAddr;
    }

    return (*this);
}

PUBLIC
IMS_BOOL SipTransportAddress::Equals(IN const SipTransportAddress& other) const
{
    if (m_nProtocol != other.m_nProtocol)
    {
        return IMS_FALSE;
    }

    if (m_nPort != other.m_nPort)
    {
        return IMS_FALSE;
    }

    if (!m_objIpAddr.Equals(other.m_objIpAddr))
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}
