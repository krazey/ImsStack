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
#ifndef SOCKET_ADDRESS_H_
#define SOCKET_ADDRESS_H_

#include "IpAddress.h"

/**
 * @brief This class defines the socket address. It consists of IP address and port number.
 */
class SocketAddress
{
public:
    inline SocketAddress() :
            m_nPort(0),
            m_objIpAddress(IpAddress::IPv6NONE)
    {
    }
    inline SocketAddress(IN const IpAddress& objIpAddress, IN IMS_SINT32 nPort) :
            m_nPort(nPort),
            m_objIpAddress(objIpAddress)
    {
    }
    inline SocketAddress(IN const AString& strIpAddress, IN IMS_SINT32 nPort) :
            m_nPort(nPort),
            m_objIpAddress(IpAddress(strIpAddress))
    {
    }
    inline SocketAddress(IN const SocketAddress& other) :
            m_nPort(other.m_nPort),
            m_objIpAddress(other.m_objIpAddress)
    {
    }
    inline ~SocketAddress() {}

public:
    SocketAddress& operator=(IN const SocketAddress& other)
    {
        if (this != &other)
        {
            m_nPort = other.m_nPort;
            m_objIpAddress = other.m_objIpAddress;
        }

        return *this;
    }

public:
    /**
     * @brief Checks if the given SocketAddress is equal or not.
     *
     * @return If it's the same address, returns IMS_TRUE.
     *        Otherwise, returns IMS_FALSE.
     */
    inline IMS_BOOL Equals(IN const SocketAddress& other) const
    {
        return (m_nPort == other.m_nPort) && m_objIpAddress.Equals(other.m_objIpAddress);
    }

    /**
     * @brief Returns an IP address of this SocketAddress.
     *
     * @return IPAddress object
     */
    inline const IpAddress& GetAddress() const { return m_objIpAddress; }

    /**
     * @brief Returns a port number of this SocketAddress.
     *
     * @return a port number
     */
    inline IMS_SINT32 GetPort() const { return m_nPort; }

    /**
     * @brief Sets an IP address of this SocketAddress.
     *
     * @param objIpAddress an IP address to be set
     */
    inline void SetAddress(IN const IpAddress& objIpAddress) { m_objIpAddress = objIpAddress; }

    /**
     * @brief Sets a port number of this SocketAddress.
     *
     * @param nPort a port number to be set
     */
    inline void SetPort(IN IMS_SINT32 nPort) { m_nPort = nPort; }

    /**
     * @brief Returns the string representation of this object.
     *
     * @return A string representation of this object
     */
    inline AString ToString() const
    {
        AString strSa;

        if (m_objIpAddress.IsIPv4Address())
        {
            strSa.Sprintf("%s:%d", m_objIpAddress.ToString().GetStr(), m_nPort);
        }
        else
        {
            strSa.Sprintf("[%s]:%d", m_objIpAddress.ToString().GetStr(), m_nPort);
        }

        return strSa;
    }

private:
    // Port number
    IMS_SINT32 m_nPort;
    // IP address (null-terminating dotted numeric IP address)
    IpAddress m_objIpAddress;
};

#endif
