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
#include "SipSocketAddress.h"

PUBLIC
SipSocketAddress::SipSocketAddress() :
        m_bSecure(IMS_FALSE),
        m_nType(SOCKET_UDP),
        m_objSockAddr(SocketAddress(IpAddress::NONE, Sip::PORT_UNSPECIFIED))
{
}

PUBLIC
SipSocketAddress::SipSocketAddress(IN const SipSocketAddress& other) :
        m_bSecure(other.m_bSecure),
        m_nType(other.m_nType),
        m_objSockAddr(other.m_objSockAddr)
{
}

PUBLIC
SipSocketAddress::SipSocketAddress(
        IN IMS_SINT32 nType, IN IMS_SINT32 nPort, IN const AString& strAddress) :
        m_bSecure(IMS_FALSE),
        m_nType(nType),
        m_objSockAddr(SocketAddress(strAddress, nPort))
{
}

PUBLIC
SipSocketAddress::SipSocketAddress(IN IMS_SINT32 nType, IN IMS_SINT32 nPort,
        IN const AString& strAddress, IN IMS_BOOL bSecure) :
        m_bSecure(bSecure),
        m_nType(nType),
        m_objSockAddr(SocketAddress(strAddress, nPort))
{
}

PUBLIC
SipSocketAddress::~SipSocketAddress() {}

PUBLIC
SipSocketAddress& SipSocketAddress::operator=(IN const SipSocketAddress& other)
{
    if (this != &other)
    {
        m_bSecure = other.m_bSecure;
        m_nType = other.m_nType;
        m_objSockAddr = other.m_objSockAddr;
    }

    return (*this);
}

PUBLIC
IMS_BOOL SipSocketAddress::Equals(IN const SipSocketAddress& other) const
{
    if (m_nType != other.m_nType)
    {
        return IMS_FALSE;
    }

    return m_objSockAddr.Equals(other.GetSocketAddress());
}
