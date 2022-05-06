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
#include "SIPPrivate.h"
#include "SIPSocketAddress.h"

PUBLIC
SIPSocketAddress::SIPSocketAddress() :
        m_bSecure(IMS_FALSE),
        m_nType(SOCKET_UDP),
        m_objSockAddr(SocketAddress(IPAddress::NONE, Sip::PORT_UNSPECIFIED))
{
}

PUBLIC
SIPSocketAddress::SIPSocketAddress(IN const SIPSocketAddress& other) :
        m_bSecure(other.m_bSecure),
        m_nType(other.m_nType),
        m_objSockAddr(other.m_objSockAddr)
{
}

PUBLIC
SIPSocketAddress::SIPSocketAddress(
        IN IMS_SINT32 nType, IN IMS_SINT32 nPort, IN const AString& strAddress) :
        m_bSecure(IMS_FALSE),
        m_nType(nType),
        m_objSockAddr(SocketAddress(strAddress, nPort))
{
}

PUBLIC
SIPSocketAddress::SIPSocketAddress(IN IMS_SINT32 nType, IN IMS_SINT32 nPort,
        IN const AString& strAddress, IN IMS_BOOL bSecure) :
        m_bSecure(bSecure),
        m_nType(nType),
        m_objSockAddr(SocketAddress(strAddress, nPort))
{
}

PUBLIC
SIPSocketAddress::~SIPSocketAddress() {}

PUBLIC
SIPSocketAddress& SIPSocketAddress::operator=(IN const SIPSocketAddress& other)
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
IMS_BOOL SIPSocketAddress::Equals(IN const SIPSocketAddress& other) const
{
    if (m_nType != other.m_nType)
    {
        return IMS_FALSE;
    }

    return m_objSockAddr.Equals(other.GetSocketAddress());
}
